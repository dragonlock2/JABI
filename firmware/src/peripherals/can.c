#include <drivers/can.h>
#include <sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/can.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_can, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, can)

#ifdef CONFIG_CAN_FD_MODE
#define MODE_FLAG CAN_MODE_FD // always enable FD if available
#else
#define MODE_FLAG 0
#endif // CONFIG_CAN_FD_MODE

#define SEND_TIMEOUT K_MSEC(500) // smaller than libjabi timeout

typedef struct {
    const struct device *dev;
    struct zcan_filter filter_std;
    struct zcan_filter filter_ext;
    int std_id;
    int ext_id;
    struct k_msgq *msgq;
} can_dev_data_t;

#define GEN_CAN_MSGQ(node_id, prop, idx) \
    CAN_MSGQ_DEFINE(can_msgq##idx, CONFIG_JABI_CAN_BUFFER_SIZE);

#define GEN_CAN_DEV_DATA(node_id, prop, idx)                      \
    {                                                             \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)), \
        .filter_std = {                                           \
            .id_type = CAN_STANDARD_IDENTIFIER,                   \
            .id_mask = 0,                                         \
            .rtr_mask = 0,                                        \
        },                                                        \
        .filter_ext = {                                           \
            .id_type = CAN_EXTENDED_IDENTIFIER,                   \
            .id_mask = 0,                                         \
            .rtr_mask = 0,                                        \
        },                                                        \
        .msgq = &can_msgq##idx,                                   \
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
    can->std_id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_std);
    can->ext_id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_ext);
    if (can->std_id == -ENOSPC || can->ext_id == -ENOSPC) {
        LOG_ERR("failed to add filters for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
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
    can_remove_rx_filter(can->dev, can->std_id);
    can_remove_rx_filter(can->dev, can->ext_id);

    can->filter_std.id       = args->id;
    can->filter_std.id_mask  = args->id_mask;
    can->filter_std.rtr      = args->rtr ? CAN_REMOTEREQUEST : CAN_DATAFRAME;
    can->filter_std.rtr_mask = args->rtr_mask ? 1 : 0;
    can->filter_ext.id       = args->id;
    can->filter_ext.id_mask  = args->id_mask;
    can->filter_ext.rtr      = args->rtr ? CAN_REMOTEREQUEST : CAN_DATAFRAME;
    can->filter_ext.rtr_mask = args->rtr_mask ? 1 : 0;

    if (args->id_mask <= CAN_STD_ID_MASK) {
        can->std_id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_std);
    }
    can->ext_id = can_add_rx_filter_msgq(can->dev, can->msgq, &can->filter_ext);
    if (can->std_id == -ENOSPC || can->ext_id == -ENOSPC) {
        LOG_ERR("failed to change filters for can%d", idx);
        return JABI_PERIPHERAL_ERR;
    }

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
    if (can_set_mode(can->dev, mode | MODE_FLAG)) {
        LOG_ERR("failed to set mode for can%d", idx);
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

static void can_write_cb(const struct device *dev, int error, void* user_data) {}

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

    struct zcan_frame msg = {
        .id      = args->id,
        .id_type = args->id_type ? CAN_EXTENDED_IDENTIFIER : CAN_STANDARD_IDENTIFIER,
        .fd      = args->fd ? 1 : 0,
        .brs     = args->brs ? 1 : 0,
        .rtr     = args->rtr ? CAN_REMOTEREQUEST : CAN_DATAFRAME,
        .dlc     = can_bytes_to_dlc(args->data_len),
    };
    if (args->data_len != can_dlc_to_bytes(msg.dlc)) {
        LOG_WRN("data_len too small for packet, padding with zeros");
        for (int i = args->data_len; i < can_dlc_to_bytes(msg.dlc); i++) {
            args->data[i] = 0;
        }
    }
    memcpy(msg.data, args->data, can_dlc_to_bytes(msg.dlc));

    can_dev_data_t *can = &can_devs[idx];
    if (can_send(can->dev, &msg, SEND_TIMEOUT, can_write_cb, NULL)) {
        LOG_ERR("couldn't send message");
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(can_read) {
    PERIPH_FUNC_GET_RET(can, read);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    struct zcan_frame msg;
    can_dev_data_t *can = &can_devs[idx];
    if (k_msgq_get(can->msgq, &msg, K_NO_WAIT)) {
        *resp_len = 0;
        return JABI_NO_ERR;
    }

    ret->num_left = sys_cpu_to_le16(k_msgq_num_used_get(can->msgq));
    ret->id       = sys_cpu_to_le32(msg.id);
    ret->id_type  = msg.id_type == CAN_EXTENDED_IDENTIFIER;
    ret->fd       = msg.fd;
    ret->brs      = msg.brs;
    ret->rtr      = msg.rtr == CAN_REMOTEREQUEST;
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
