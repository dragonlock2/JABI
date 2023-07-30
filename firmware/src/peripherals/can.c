#include <zephyr/drivers/can.h>
#include <zephyr/sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/can.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(periph_can, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, can)

#ifdef CONFIG_CAN_FD_MODE
#define MODE_FLAG CAN_MODE_FD // always enable FD if available
#else
#define MODE_FLAG 0
#endif // CONFIG_CAN_FD_MODE

#define SEND_TIMEOUT K_MSEC(500) // smaller than libjabi timeout

typedef struct {
    struct can_filter filter;
    int id;
} can_filter_data_t;

typedef struct {
    const struct device *dev;
    can_filter_data_t filter_std;
    can_filter_data_t filter_ext;
    can_filter_data_t filter_std_fd;
    can_filter_data_t filter_ext_fd;
    struct k_msgq *msgq;
    struct k_sem tx_lock;
    bool tx_was_busy;
} can_dev_data_t;

#define GEN_CAN_MSGQ(node_id, prop, idx) \
    CAN_MSGQ_DEFINE(can_msgq##idx, CONFIG_JABI_CAN_BUFFER_SIZE);

#define GEN_CAN_DEV_DATA(node_id, prop, idx)                                             \
    {                                                                                    \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)),                        \
        .filter_std.filter = {                                                           \
            .id = 0,                                                                     \
            .mask = 0,                                                                   \
            .flags = CAN_FILTER_RTR | CAN_FILTER_DATA,                                   \
        },                                                                               \
        .filter_ext.filter = {                                                           \
            .id = 0,                                                                     \
            .mask = 0,                                                                   \
            .flags = CAN_FILTER_RTR | CAN_FILTER_DATA | CAN_FILTER_IDE,                  \
        },                                                                               \
        .filter_std_fd.filter = {                                                        \
            .id = 0,                                                                     \
            .mask = 0,                                                                   \
            .flags = CAN_FILTER_RTR | CAN_FILTER_DATA | CAN_FILTER_FDF,                  \
        },                                                                               \
        .filter_ext_fd.filter = {                                                        \
            .id = 0,                                                                     \
            .mask = 0,                                                                   \
            .flags = CAN_FILTER_RTR | CAN_FILTER_DATA | CAN_FILTER_IDE | CAN_FILTER_FDF, \
        },                                                                               \
        .msgq = &can_msgq##idx,                                                          \
        .tx_was_busy = false,                                                            \
    },

DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, can, GEN_CAN_MSGQ);

static can_dev_data_t can_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, can, GEN_CAN_DEV_DATA)
};

static int can_init(uint16_t idx) {
    can_dev_data_t *can = &can_devs[idx];
    if (can_set_mode(can->dev, CAN_MODE_NORMAL | MODE_FLAG)) {
        LOG_ERR("failed to set mode for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    if (can_start(can->dev) == -EIO) {
        LOG_ERR("failed to start can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    can->filter_std.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_std.filter);
    can->filter_ext.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_ext.filter);
    if (can->filter_std.id == -ENOSPC || can->filter_ext.id == -ENOSPC) {
        LOG_ERR("failed to add filters for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
#ifdef CONFIG_CAN_FD_MODE
    can->filter_std_fd.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_std_fd.filter);
    can->filter_ext_fd.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_ext_fd.filter);
    if (can->filter_std_fd.id == -ENOSPC || can->filter_ext_fd.id == -ENOSPC) {
        LOG_ERR("failed to add FD filters for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
#endif
    k_sem_init(&can->tx_lock, 0, 1);
    return JABI_NO_ERR;
}

static void *can_get_dev(uint16_t idx) {
    return (void*) can_devs[idx].dev;
}

PERIPH_FUNC_DEF(can_set_filter) {
    PERIPH_FUNC_GET_ARGS(can, set_filter);
    PERIPH_FUNC_CHECK_ARGS_LEN(can, set_filter);

    args->id = sys_le32_to_cpu(args->id);
    args->id_mask = sys_le32_to_cpu(args->id_mask);

    can_set_filter_req_t *tmp = args; // LOG_DBG uses the name args...
    LOG_DBG("(id=0x%x,id_mask=0x%x,rtr=%d,rtr_mask=%d)",
        tmp->id, tmp->id_mask, tmp->rtr, tmp->rtr_mask);

    can_dev_data_t *can = &can_devs[idx];
    can_remove_rx_filter(can->dev, can->filter_std.id);
    can_remove_rx_filter(can->dev, can->filter_ext.id);
#ifdef CONFIG_CAN_FD_MODE
    can_remove_rx_filter(can->dev, can->filter_std_fd.id);
    can_remove_rx_filter(can->dev, can->filter_ext_fd.id);
#endif

    uint8_t flags = 0;
    if (args->rtr_mask) {
        flags = args->rtr ? CAN_FILTER_RTR : CAN_FILTER_DATA;
    } else {
        flags = CAN_FILTER_RTR | CAN_FILTER_DATA;
    }
    can->filter_std.filter.id       = args->id;
    can->filter_ext.filter.id       = args->id;
    can->filter_std_fd.filter.id    = args->id;
    can->filter_ext_fd.filter.id    = args->id;
    can->filter_std.filter.mask     = args->id_mask;
    can->filter_ext.filter.mask     = args->id_mask;
    can->filter_std_fd.filter.mask  = args->id_mask;
    can->filter_ext_fd.filter.mask  = args->id_mask;
    can->filter_std.filter.flags    = flags;
    can->filter_ext.filter.flags    = flags | CAN_FILTER_IDE;
    can->filter_std_fd.filter.flags = flags | CAN_FILTER_FDF;
    can->filter_ext_fd.filter.flags = flags | CAN_FILTER_IDE | CAN_FILTER_FDF;

    if (args->id_mask <= CAN_STD_ID_MASK) {
        can->filter_std.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_std.filter);
    }
    can->filter_ext.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_ext.filter);
    if (can->filter_std.id == -ENOSPC || can->filter_ext.id == -ENOSPC) {
        LOG_ERR("failed to change filters for can%d, old filter also removed", idx);
        return JABI_PERIPHERAL_ERR;
    }
#ifdef CONFIG_CAN_FD_MODE
    if (args->id_mask <= CAN_STD_ID_MASK) {
        can->filter_std_fd.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_std_fd.filter);
    }
    can->filter_ext_fd.id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_ext_fd.filter);
    if (can->filter_std_fd.id == -ENOSPC || can->filter_ext_fd.id == -ENOSPC) {
        LOG_ERR("failed to change FD filters for can%d, old filter also removed", idx);
        return JABI_PERIPHERAL_ERR;
    }
#endif // CONFIG_CAN_FD_MODE

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(can_set_rate) {
    PERIPH_FUNC_GET_ARGS(can, set_rate);
    PERIPH_FUNC_CHECK_ARGS_LEN(can, set_rate);

    args->bitrate      = sys_le32_to_cpu(args->bitrate);
    args->bitrate_data = sys_le32_to_cpu(args->bitrate_data);

    LOG_DBG("(bitrate=%d,bitrate_data=%d)", args->bitrate, args->bitrate_data);

    can_dev_data_t *can = &can_devs[idx];
    if (can_stop(can->dev) == -EIO) { // note stays stopped if invalid bitrates
        LOG_ERR("failed to stop can %d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    if (can_set_bitrate(can->dev, args->bitrate)) {
        LOG_ERR("failed to set bitrate for can %d", idx);
        return JABI_PERIPHERAL_ERR;
    }
#ifdef CONFIG_CAN_FD_MODE
    if (can_set_bitrate_data(can->dev, args->bitrate_data)) {
        LOG_ERR("failed to set data bitrate for can %d", idx);
        return JABI_PERIPHERAL_ERR;
    }
#endif // CONFIG_CAN_FD_MODE
    if (can_start(can->dev) == -EIO) {
        LOG_ERR("failed to restart can %d", idx);
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(can_set_style) {
    PERIPH_FUNC_GET_ARGS(can, set_style);
    PERIPH_FUNC_CHECK_ARGS_LEN(can, set_style);

    LOG_DBG("(mode=%d)", args->mode);

    can_mode_t mode;
    switch (args->mode) {
        case 0: mode = CAN_MODE_NORMAL;     break;
        case 1: mode = CAN_MODE_LOOPBACK;   break;
        case 2: mode = CAN_MODE_LISTENONLY; break;
        default:
            LOG_ERR("invalid mode");
            return JABI_INVALID_ARGS_ERR;
    }
    can_dev_data_t *can = &can_devs[idx];
    if (can_stop(can->dev) == -EIO) { // note stays stopped if fails
        LOG_ERR("failed to stop can %d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    if (can_set_mode(can->dev, mode | MODE_FLAG)) {
        LOG_ERR("failed to set mode for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    if (can_start(can->dev) == -EIO) {
        LOG_ERR("failed to restart can %d", idx);
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(can_state) {
    PERIPH_FUNC_GET_RET(can, state);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    enum can_state state;
    struct can_bus_err_cnt err_cnt;
    can_dev_data_t *can = &can_devs[idx];
    if (can_get_state(can->dev, &state, &err_cnt)) {
        LOG_ERR("failed to get state for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }

    ret->state = state;
    ret->tx_err_cnt = err_cnt.tx_err_cnt;
    ret->rx_err_cnt = err_cnt.rx_err_cnt;
    *resp_len = sizeof(can_state_resp_t);
    return JABI_NO_ERR;
}

static void can_write_cb(const struct device *dev, int error, void* user_data) {
    if (error) { // shouldn't happen w/ auto recovery
        LOG_ERR("message send error %d?!", error);
    }
    can_dev_data_t *can = (can_dev_data_t*) user_data;
    k_sem_give(&can->tx_lock);
}

PERIPH_FUNC_DEF(can_write) {
    PERIPH_FUNC_GET_ARGS(can, write);
    args->id = sys_le32_to_cpu(args->id);

    if (req_len != (sizeof(can_write_req_t) + (args->rtr ? 0 : args->data_len))) {
        LOG_ERR("invalid amount of data provided");
        return JABI_INVALID_ARGS_FORMAT_ERR;
    }

    can_write_req_t *tmp = args;
    LOG_DBG("(id=0x%x,id_type=%d,fd=%d,brs=%d,rtr=%d,data_len=%d)",
        tmp->id, tmp->id_type, tmp->fd, tmp->brs, tmp->rtr, tmp->data_len);
    if (!args->rtr) {
        LOG_HEXDUMP_DBG(tmp->data, tmp->data_len, "data=");
    }

    if (args->data_len > CAN_MAX_DLEN) {
        LOG_ERR("data_len too large");
        return JABI_NOT_SUPPORTED_ERR;
    }
    if (args->id_type && args->id > CAN_EXT_ID_MASK) {
        LOG_WRN("id 0x%x too large for extended id", args->id);
    } else if (args->id_type == 0 && args->id > CAN_STD_ID_MASK) {
        LOG_WRN("id 0x%x too large for standard id", args->id);
    }

    struct can_frame msg = {
        .id    = args->id,
        .dlc   = can_bytes_to_dlc(args->data_len),
        .flags = (args->id_type ? CAN_FRAME_IDE : 0) |
                 (args->rtr     ? CAN_FRAME_RTR : 0) |
                 (args->fd      ? CAN_FRAME_FDF : 0) |
                 (args->brs     ? CAN_FRAME_BRS : 0),
    };
    if (args->data_len != can_dlc_to_bytes(msg.dlc)) {
        LOG_WRN("data_len too small for packet, padding with zeros");
        for (int i = args->data_len; i < can_dlc_to_bytes(msg.dlc); i++) {
            args->data[i] = 0;
        }
    }
    memcpy(msg.data, args->data, can_dlc_to_bytes(msg.dlc));

    can_dev_data_t *can = &can_devs[idx];
    if (can->tx_was_busy) {
        if (k_sem_take(&can->tx_lock, K_NO_WAIT)) {
            return JABI_BUSY_ERR;
        }
    }
    if (can_send(can->dev, &msg, K_NO_WAIT, can_write_cb, can)) {
        LOG_ERR("couldn't send message");
        return JABI_PERIPHERAL_ERR;
    }
    // wait for completion to ensure FIFO sending
    if (k_sem_take(&can->tx_lock, SEND_TIMEOUT)) {
        LOG_ERR("message didn't send in time (still in queue tho)");
        can->tx_was_busy = true;
        return JABI_BUSY_ERR;
    }
    can->tx_was_busy = false;
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(can_read) {
    PERIPH_FUNC_GET_RET(can, read);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    struct can_frame msg;
    can_dev_data_t *can = &can_devs[idx];
    if (k_msgq_get(can->msgq, &msg, K_NO_WAIT)) {
        *resp_len = 0;
        return JABI_NO_ERR;
    }

    ret->num_left = sys_cpu_to_le16(k_msgq_num_used_get(can->msgq));
    ret->id       = sys_cpu_to_le32(msg.id);
    ret->id_type  = msg.flags & CAN_FRAME_IDE;
    ret->fd       = msg.flags & CAN_FRAME_FDF;
    ret->brs      = msg.flags & CAN_FRAME_BRS;
    ret->rtr      = msg.flags & CAN_FRAME_RTR;
    ret->data_len = can_dlc_to_bytes(msg.dlc);
    *resp_len = sizeof(can_read_resp_t);
    if (!ret->rtr) {
        memcpy(ret->data, msg.data, ret->data_len);
        *resp_len += ret->data_len;
    }
    return JABI_NO_ERR;
}

static const periph_func_t can_periph_fns[] = {
    can_set_filter,
    can_set_rate,
    can_set_style,
    can_state,
    can_write,
    can_read,
};

const struct periph_api_t can_periph_api = {
    .init = can_init,
    .get_dev = can_get_dev,
    .fns = can_periph_fns,
    .num_fns = ARRAY_SIZE(can_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, can),
    .name = "can",
};

#else

const struct periph_api_t can_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "can",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, can)
