#include <zephyrboards/drivers/lin.h>
#include <zephyr/sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/lin.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(periph_lin, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, lin)

typedef struct {
    const struct device *dev;
    uint8_t mode;
    struct {
        uint8_t type;
        uint8_t len;
    } filters[LIN_NUM_ID];
    struct {
        bool active;
        struct lin_frame msg;
    } tx_msgs[LIN_NUM_ID];
    struct k_msgq *rx_msgs;
    lin_status_resp_t status;
    struct k_sem msg_lock;
    struct k_sem cmd_lock;
} lin_dev_data_t;

#define GEN_LIN_MSGQS(node_id, prop, idx) \
    K_MSGQ_DEFINE(lin_rx_msgq##idx, sizeof(struct lin_frame), CONFIG_JABI_LIN_BUFFER_SIZE, 4);

#define GEN_LIN_DEV_DATA(node_id, prop, idx)                      \
    {                                                             \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)), \
        .rx_msgs = &lin_rx_msgq##idx,                             \
    },

DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, lin, GEN_LIN_MSGQS);

static lin_dev_data_t lin_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, lin, GEN_LIN_DEV_DATA)
};

static int lin_header_cb(const struct device *dev, struct lin_frame *msg, void *arg) {
    // should have locks for every filter and tx msg, but this works well enough
    lin_dev_data_t *lin = (lin_dev_data_t*) arg;
    if (k_sem_take(&lin->msg_lock, K_NO_WAIT)) {
        return LIN_ACTION_NONE;
    }
    int ret = LIN_ACTION_NONE;
    if (lin->tx_msgs[msg->id].active) {
        lin->tx_msgs[msg->id].active = false;
        lin->status.id = msg->id;
        lin->status.retcode = 0;
        *msg = lin->tx_msgs[msg->id].msg;
        ret = LIN_ACTION_SEND;
    } else {
        lin->status.id = msg->id;
        lin->status.retcode = 0;
        msg->type = lin->filters[msg->id].type;
        msg->len = lin->filters[msg->id].len;
        ret = LIN_ACTION_RECEIVE;
    }
    k_sem_give(&lin->msg_lock);
    return ret;
}

static void lin_tx_cb(const struct device *dev, int error, void *arg) {
    lin_dev_data_t *lin = (lin_dev_data_t*) arg;
    lin->status.retcode = error;
    if (lin->mode == 0) {
        k_sem_give(&lin->cmd_lock);
    }
}

static void lin_rx_cb(const struct device *dev, int error, const struct lin_frame *msg, void *arg) {
    lin_dev_data_t *lin = (lin_dev_data_t*) arg;
    lin->status.retcode = error;
    if (!error) {
        if (k_msgq_put(lin->rx_msgs, msg, K_NO_WAIT)) {
            LOG_ERR("overflow, unable to store message %d", msg->id);
        }
    }
    if (lin->mode == 0) {
        k_sem_give(&lin->cmd_lock);
    }
}

static int lin_init(uint16_t idx) {
    lin_dev_data_t *lin = &lin_devs[idx];
    lin->mode = LIN_MODE_RESPONDER;
    if (lin_set_mode(lin->dev, LIN_MODE_RESPONDER) ||
        lin_set_header_callback(lin->dev, lin_header_cb, lin) ||
        lin_set_tx_callback(lin->dev, lin_tx_cb, lin) ||
        lin_set_rx_callback(lin->dev, lin_rx_cb, lin)) {
        LOG_ERR("failed to configure lin%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    for (int i = 0; i < LIN_NUM_ID; i++) { // default to sniffing everything
        lin->filters[i].type = LIN_CHECKSUM_AUTO;
        lin->filters[i].len = 0;
    }
    for (int i = 0; i < LIN_NUM_ID; i++) {
        lin->tx_msgs[i].active = false;
    }
    lin->status.id = 0xFF;
    lin->status.retcode = 0;
    k_sem_init(&lin->msg_lock, 1, 1);
    k_sem_init(&lin->cmd_lock, 0, 1);
    return JABI_NO_ERR;
}

static void *lin_get_dev(uint16_t idx) {
    return (void*) lin_devs[idx].dev;
}

PERIPH_FUNC_DEF(lin_set_mode_j) {
    PERIPH_FUNC_GET_ARGS(lin, set_mode_j);
    PERIPH_FUNC_CHECK_ARGS_LEN(lin, set_mode_j);

    LOG_DBG("(mode=%d)", args->mode);

    enum lin_mode mode;
    switch (args->mode) {
        case 0: mode = LIN_MODE_COMMANDER; break;
        case 1: mode = LIN_MODE_RESPONDER; break;
        default:
            LOG_ERR("invalid mode");
            return JABI_INVALID_ARGS_ERR;
    }
    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin_set_mode(lin->dev, mode)) {
        LOG_ERR("failed to set mode for lin%d", idx);
        return JABI_PERIPHERAL_ERR;
    }

    if (lin->mode == 1 && args->mode == 0) { // responder->commander transition
        // purge all messages
        for (int i = 0; i < LIN_NUM_ID; i++) {
            lin->tx_msgs[i].active = false;
        }
        k_msgq_purge(lin->rx_msgs);
    }
    lin->mode = args->mode;

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_set_rate) {
    PERIPH_FUNC_GET_ARGS(lin, set_rate);
    PERIPH_FUNC_CHECK_ARGS_LEN(lin, set_rate);

    args->bitrate = sys_le32_to_cpu(args->bitrate);

    LOG_DBG("(bitrate=%d)", args->bitrate);

    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin_set_bitrate(lin->dev, args->bitrate)) {
        LOG_ERR("failed to set bitrate for lin %d", idx);
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_set_filter) {
    PERIPH_FUNC_GET_ARGS(lin, set_filter);
    PERIPH_FUNC_CHECK_ARGS_LEN(lin, set_filter);

    lin_set_filter_req_t *tmp = args;
    LOG_DBG("(id=%d,checksum_type=%d,data_len=%d)",
        tmp->id, tmp->checksum_type, tmp->data_len);

    if (args->id >= LIN_NUM_ID || args->data_len > LIN_MAX_DLEN) {
        LOG_ERR("invalid id or data");
        return JABI_INVALID_ARGS_ERR;
    }

    enum lin_checksum type;
    switch (args->checksum_type) {
        case 0: type = LIN_CHECKSUM_CLASSIC;  break;
        case 1: type = LIN_CHECKSUM_ENHANCED; break;
        case 2: type = LIN_CHECKSUM_AUTO;     break;
        default:
            LOG_ERR("invalid checksum type");
            return JABI_INVALID_ARGS_ERR;
    }

    lin_dev_data_t *lin = &lin_devs[idx];
    k_sem_take(&lin->msg_lock, K_FOREVER);
    lin->filters[args->id].type = type;
    lin->filters[args->id].len = args->data_len;
    k_sem_give(&lin->msg_lock);

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_mode) {
    PERIPH_FUNC_GET_RET(lin, mode);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    lin_dev_data_t *lin = &lin_devs[idx];

    ret->mode = lin->mode;
    *resp_len = sizeof(lin_mode_resp_t);
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_status) {
    PERIPH_FUNC_GET_RET(lin, status);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    // not isr safe, but functional enough
    // might return no error while send/receive packet, need to query twice w/ delay
    lin_dev_data_t *lin = &lin_devs[idx];
    ret->id = lin->status.id;
    ret->retcode = sys_cpu_to_le16(lin->status.retcode);
    *resp_len = sizeof(lin_status_resp_t);
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_write) {
    PERIPH_FUNC_GET_ARGS(lin, write);

    if (req_len <= sizeof(lin_write_req_t) || req_len > (sizeof(lin_write_req_t) + LIN_MAX_DLEN)) {
        LOG_ERR("must provide at 1-8 data bytes");
        return JABI_INVALID_ARGS_FORMAT_ERR;
    }
    uint8_t data_len = req_len - sizeof(lin_write_req_t);

    LOG_DBG("(id=%d,checksum_type=%d)", args->id, args->checksum_type);
    LOG_HEXDUMP_DBG(args->data, data_len, "data=");

    if (args->id >= LIN_NUM_ID) {
        LOG_ERR("invalid id");
        return JABI_INVALID_ARGS_ERR;
    }
    struct lin_frame msg = {
        .id = args->id,
        .len = data_len,
    };
    switch (args->checksum_type) {
        case 0: msg.type = LIN_CHECKSUM_CLASSIC;  break;
        case 1: msg.type = LIN_CHECKSUM_ENHANCED; break;
        default:
            LOG_ERR("invalid checksum type");
            return JABI_INVALID_ARGS_ERR;
    }
    memcpy(msg.data, args->data, data_len);

    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin->mode == 0) { // commander
        if (lin_send(lin->dev, &msg)) {
            LOG_ERR("failed sending frame, likely invalid args??");
            return JABI_INVALID_ARGS_ERR;
        }
        k_sem_take(&lin->cmd_lock, K_FOREVER); // bounded latency
        if (lin->status.retcode) {
            LOG_ERR("error sending frame");
            return JABI_PERIPHERAL_ERR;
        }
    } else { // responder
        k_sem_take(&lin->msg_lock, K_FOREVER);
        lin->tx_msgs[msg.id].active = true;
        lin->tx_msgs[msg.id].msg = msg;
        k_sem_give(&lin->msg_lock);
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_read) {
    PERIPH_FUNC_GET_ARGS(lin, read);
    PERIPH_FUNC_GET_RET(lin, read);
    PERIPH_FUNC_CHECK_ARGS_LEN(lin, read);

    LOG_DBG("(id=%d)", args->id);

    struct lin_frame msg;
    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin->mode == 0) { // commander
        if (args->id >= LIN_NUM_ID) {
            LOG_ERR("invalid id");
            return JABI_INVALID_ARGS_ERR;
        }
        if (lin_receive(lin->dev, args->id, lin->filters[args->id].type,
                lin->filters[args->id].len)) {
            LOG_ERR("failed sending frame, likely invalid args??");
            return JABI_INVALID_ARGS_ERR;
        }
        k_sem_take(&lin->cmd_lock, K_FOREVER); // bounded latency
        if (lin->status.retcode) {
            LOG_ERR("timeout receiving frame");
            return JABI_TIMEOUT_ERR;
        }
        if (k_msgq_get(lin->rx_msgs, &msg, K_NO_WAIT)) { // guaranteed if no error
            LOG_ERR("no received message, is filter correct?");
            return JABI_PERIPHERAL_ERR;
        }
    } else { // responder
        if (k_msgq_get(lin->rx_msgs, &msg, K_NO_WAIT)) {
            *resp_len = 0;
            return JABI_NO_ERR;
        }
    }

    ret->num_left = sys_cpu_to_le16(k_msgq_num_used_get(lin->rx_msgs));
    ret->id = msg.id;
    ret->checksum_type = msg.type == LIN_CHECKSUM_CLASSIC ? 0 : 1;
    memcpy(ret->data, msg.data, msg.len);
    *resp_len = sizeof(lin_read_resp_t) + msg.len;
    return JABI_NO_ERR;
}

static const periph_func_t lin_periph_fns[] = {
    lin_set_mode_j,
    lin_set_rate,
    lin_set_filter,
    lin_mode,
    lin_status,
    lin_write,
    lin_read,
};

const struct periph_api_t lin_periph_api = {
    .init = lin_init,
    .get_dev = lin_get_dev,
    .fns = lin_periph_fns,
    .num_fns = ARRAY_SIZE(lin_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, lin),
    .name = "lin",
};

#else // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, lin)

const struct periph_api_t lin_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "lin",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, lin)
