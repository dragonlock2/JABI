#include <zephyrboards/drivers/lin.h>
#include <sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/lin.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_lin, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, lin)

typedef struct {
    const struct device *dev;
    uint8_t mode;
    int filters[LIN_NUM_ID];
    struct k_msgq *results;
    struct k_msgq *msgs;
} lin_dev_data_t;

#define GEN_LIN_MSGQS(node_id, prop, idx)                                                      \
    K_MSGQ_DEFINE(lin_results##idx, sizeof(lin_status_resp_t), CONFIG_JABI_LIN_BUFFER_SIZE, 4); \
    LIN_MSGQ_DEFINE(lin_msgq##idx, CONFIG_JABI_LIN_BUFFER_SIZE);

#define GEN_LIN_DEV_DATA(node_id, prop, idx)                      \
    {                                                             \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)), \
        .mode = 1,                                                \
        .results = &lin_results##idx,                             \
        .msgs = &lin_msgq##idx,                                   \
    },

DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, lin, GEN_LIN_MSGQS);

static lin_dev_data_t lin_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, lin, GEN_LIN_DEV_DATA)
};

static int lin_init(uint16_t idx) {
    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin_set_mode(lin->dev, lin->mode)) {
        LOG_ERR("failed to set mode for lin%d", idx);
        return JABI_PERIPHERAL_ERR;
    }
    struct zlin_filter filter = { // default to sniffing everything
        .checksum_type = LIN_CHECKSUM_AUTO,
        .data_len = 0,
    };
    for (int i = 0; i < LIN_NUM_ID; i++) {
        filter.id = i;
        if ((lin->filters[i] = lin_add_rx_filter_msgq(lin->dev, lin->msgs, &filter)) < 0) {
            LOG_ERR("failed to add filter for id %d for lin%d", i, idx);
            return JABI_PERIPHERAL_ERR;
        }
    }
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
        k_msgq_purge(lin->results); // set_mode already purges pending messages
        k_msgq_purge(lin->msgs);
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

    LOG_DBG("(id=%d,checksum_type=%d,data_len=%d)",
        args->id, args->checksum_type, args->data_len);

    if (args->id >= LIN_NUM_ID || args->data_len > LIN_MAX_DLEN) {
        LOG_ERR("invalid id or data");
        return JABI_INVALID_ARGS_ERR;
    }

    struct zlin_filter filter = {
        .id = args->id,
        .data_len = args->data_len,
    };
    switch (args->checksum_type) {
        case 0: filter.checksum_type = LIN_CHECKSUM_CLASSIC;  break;
        case 1: filter.checksum_type = LIN_CHECKSUM_ENHANCED; break;
        case 2: filter.checksum_type = LIN_CHECKSUM_AUTO;     break;
        default:
            LOG_ERR("invalid checksum type");
            return JABI_INVALID_ARGS_ERR;
    }

    lin_dev_data_t *lin = &lin_devs[idx];
    lin_remove_rx_filter(lin->dev, lin->filters[args->id]);
    if ((lin->filters[args->id] = lin_add_rx_filter_msgq(lin->dev, lin->msgs, &filter)) < 0) {
        LOG_ERR("failed to change filters for lin%d, old filter also removed", idx);
        return JABI_PERIPHERAL_ERR;
    }

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

    lin_dev_data_t *lin = &lin_devs[idx];
    lin_status_resp_t stat;
    if (k_msgq_get(lin->results, &stat, K_NO_WAIT)) {
        stat.id = 0xFF;
        stat.retcode = 0;
    }

    ret->id = stat.id;
    ret->retcode = sys_cpu_to_le16(stat.retcode);
    *resp_len = sizeof(lin_status_resp_t);
    return JABI_NO_ERR;
}

static void lin_write_cb(const struct device *dev, int error, void* user_data) {
    lin_dev_data_t *lin = CONTAINER_OF(dev, lin_dev_data_t, dev);
    lin_status_resp_t stat = {
        .id = (uint8_t) (int) user_data,
        .retcode = error,
    };
    if (k_msgq_put(lin->results, &stat, K_NO_WAIT)) {
        LOG_ERR("overflow, unable to store status of messsage %d %d",
            stat.id, stat.retcode);
    }
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
    struct zlin_frame msg = {
        .id = args->id,
        .data_len = data_len,
    };
    switch (args->checksum_type) {
        case 0: msg.checksum_type = LIN_CHECKSUM_CLASSIC;  break;
        case 1: msg.checksum_type = LIN_CHECKSUM_ENHANCED; break;
        default:
            LOG_ERR("invalid checksum type");
            return JABI_INVALID_ARGS_ERR;
    }
    memcpy(msg.data, args->data, data_len);

    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin->mode == 0) { // commander
        if (lin_send(lin->dev, &msg, K_FOREVER, lin_write_cb, (void*) (int) msg.id)) { // bounded latency
            LOG_ERR("failed sending frame, likely invalid args??");
            return JABI_INVALID_ARGS_ERR;
        }
        lin_status_resp_t stat;
        k_msgq_get(lin->results, &stat, K_FOREVER); // bounded latency
        if (stat.retcode) {
            LOG_ERR("error sending frame");
            return JABI_PERIPHERAL_ERR;
        }
    } else { // responder
        int ret = lin_send(lin->dev, &msg, K_NO_WAIT, lin_write_cb, (void*) (int) msg.id);
        if (ret == -EAGAIN) {
            return JABI_BUSY_ERR;
        } else if (ret) {
            LOG_ERR("failed sending frame, likely invalid args??");
            return JABI_INVALID_ARGS_ERR;
        }
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(lin_read) {
    PERIPH_FUNC_GET_ARGS(lin, read);
    PERIPH_FUNC_GET_RET(lin, read);
    PERIPH_FUNC_CHECK_ARGS_LEN(lin, read);

    LOG_DBG("id=%d", args->id);

    struct zlin_frame msg;
    lin_dev_data_t *lin = &lin_devs[idx];
    if (lin->mode == 0) { // commander
        if (args->id >= LIN_NUM_ID) {
            LOG_ERR("invalid id");
            return JABI_INVALID_ARGS_ERR;
        }
        if (lin_receive(lin->dev, args->id, K_FOREVER, lin_write_cb, (void*) (int) args->id)) { // bounded latency
            LOG_ERR("failed sending frame, likely invalid args??");
            return JABI_INVALID_ARGS_ERR;
        }
        lin_status_resp_t stat;
        k_msgq_get(lin->results, &stat, K_FOREVER); // bounded latency
        if (stat.retcode) {
            LOG_ERR("error receiving frame");
            return JABI_PERIPHERAL_ERR;
        }
        k_msgq_get(lin->msgs, &msg, K_FOREVER); // bounded latency
    } else { // responder
        if (k_msgq_get(lin->msgs, &msg, K_NO_WAIT)) {
            *resp_len = 0;
            return JABI_NO_ERR;
        }
    }

    ret->num_left = sys_cpu_to_le16(k_msgq_num_used_get(lin->msgs));
    ret->id = msg.id;
    ret->checksum_type = msg.checksum_type == LIN_CHECKSUM_CLASSIC ? 0 : 1;
    memcpy(ret->data, msg.data, msg.data_len);
    *resp_len = sizeof(lin_read_resp_t) + msg.data_len;
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
