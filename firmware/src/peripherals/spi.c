#include <zephyr/drivers/spi.h>
#include <zephyr/sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/spi.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(periph_spi, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, spi)

#define GEN_SPI_DEV_DATA(node_id, prop, idx)                                      \
    {                                                                             \
        .bus = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)),                 \
        .configs[0] = {                                                           \
            .frequency = 1000000,                                                 \
            .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_LINES_SINGLE, \
            .slave = 0,                                                           \
            .cs = {{0}},                                                          \
        },                                                                        \
        .curr_cfg = 0,                                                            \
        .stale = false,                                                           \
    },

// SPI API uses pointer comparison to know when to update the config
typedef struct {
    const struct device *bus;
    struct spi_config configs[2];
    int curr_cfg;
    bool stale;
} spi_dev_data_t;

static spi_dev_data_t spi_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, spi, GEN_SPI_DEV_DATA)
};

static int spi_init(uint16_t idx) {
    spi_devs[idx].configs[1] = spi_devs[idx].configs[0];
    return JABI_NO_ERR;
}

static void *spi_get_dev(uint16_t idx) {
    return (void*) spi_devs[idx].bus;
}

PERIPH_FUNC_DEF(spi_set_freq) {
    PERIPH_FUNC_GET_ARGS(spi, set_freq);
    PERIPH_FUNC_CHECK_ARGS_LEN(spi, set_freq);
    args->freq = sys_le32_to_cpu(args->freq);

    LOG_DBG("(freq=%d)", args->freq);

    spi_devs[idx].configs[0].frequency = args->freq;

    spi_devs[idx].configs[1].frequency = spi_devs[idx].configs[0].frequency;
    spi_devs[idx].stale = true;
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(spi_set_mode) {
    PERIPH_FUNC_GET_ARGS(spi, set_mode);
    PERIPH_FUNC_CHECK_ARGS_LEN(spi, set_mode);

    LOG_DBG("(mode=%d)", args->mode);

    spi_devs[idx].configs[0].operation &= ~SPI_MODE_MASK;
    switch (args->mode) {
        case 0:
            break;
        case 1:
            spi_devs[idx].configs[0].operation |= SPI_MODE_CPHA;
            break;
        case 2:
            spi_devs[idx].configs[0].operation |= SPI_MODE_CPOL;
            break;
        case 3:
            spi_devs[idx].configs[0].operation |= SPI_MODE_CPOL;
            spi_devs[idx].configs[0].operation |= SPI_MODE_CPHA;
            break;
        default:
            LOG_ERR("invalid mode");
            return JABI_INVALID_ARGS_ERR;
    }

    spi_devs[idx].configs[1].operation = spi_devs[idx].configs[0].operation;
    spi_devs[idx].stale = true;
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(spi_set_bitorder) {
    PERIPH_FUNC_GET_ARGS(spi, set_bitorder);
    PERIPH_FUNC_CHECK_ARGS_LEN(spi, set_bitorder);

    LOG_DBG("(order=%d)", args->order);

    if (args->order) {
        spi_devs[idx].configs[0].operation &= ~SPI_TRANSFER_LSB;
    } else {
        spi_devs[idx].configs[0].operation |= SPI_TRANSFER_LSB;
    }

    spi_devs[idx].configs[1].operation = spi_devs[idx].configs[0].operation;
    spi_devs[idx].stale = true;
    *resp_len = 0;
    return JABI_NO_ERR;
}

static inline void spi_update_config(uint16_t idx) {
    if (spi_devs[idx].stale) {
        spi_devs[idx].curr_cfg = spi_devs[idx].curr_cfg == 0 ? 1 : 0;
        spi_devs[idx].stale = false;
    }
}

PERIPH_FUNC_DEF(spi_write_j) {
    PERIPH_FUNC_GET_ARGS(spi, write_j);

    LOG_DBG("()");
    LOG_HEXDUMP_DBG(args, req_len, "data=");

    spi_update_config(idx);
    spi_dev_data_t *spi = &spi_devs[idx];
    struct spi_buf buf = { .buf = args, .len = req_len };
    struct spi_buf_set set = { .buffers = &buf, .count = 1};
    if (spi_write(spi->bus, &spi->configs[spi->curr_cfg], &set)) {
        LOG_ERR("failed to write");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(spi_read_j) {
    PERIPH_FUNC_GET_ARGS(spi, read_j);
    PERIPH_FUNC_GET_RET(spi, read_j);
    PERIPH_FUNC_CHECK_ARGS_LEN(spi, read_j);
    args->data_len = sys_le16_to_cpu(args->data_len);

    if (args->data_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("requested read too long");
        return JABI_INVALID_ARGS_ERR;
    }

    LOG_DBG("(data_len=%d)", args->data_len);

    spi_update_config(idx);
    spi_dev_data_t *spi = &spi_devs[idx];
    struct spi_buf buf = { .buf = ret, .len = args->data_len };
    struct spi_buf_set set = { .buffers = &buf, .count = 1 };
    if (spi_read(spi->bus, &spi->configs[spi->curr_cfg], &set)) {
        LOG_ERR("failed to write");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = args->data_len;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(spi_transceive_j) {
    PERIPH_FUNC_GET_ARGS(spi, transceive_j);
    PERIPH_FUNC_GET_RET(spi, transceive_j);

    if (req_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("input data too long");
        return JABI_INVALID_ARGS_ERR;
    }

    LOG_DBG("()");
    LOG_HEXDUMP_DBG(args, req_len, "data=");

    spi_update_config(idx);
    spi_dev_data_t *spi = &spi_devs[idx];
    struct spi_buf txbuf = { .buf = args, .len = req_len };
    struct spi_buf rxbuf = { .buf = ret, .len = req_len };
    struct spi_buf_set txset = { .buffers = &txbuf, .count = 1};
    struct spi_buf_set rxset = { .buffers = &rxbuf, .count = 1 };
    if (spi_transceive(spi->bus, &spi->configs[spi->curr_cfg], &txset, &rxset)) {
        LOG_ERR("failed to write");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = req_len;
    return JABI_NO_ERR;
}

static const periph_func_t spi_periph_fns[] = {
    spi_set_freq,
    spi_set_mode,
    spi_set_bitorder,
    spi_write_j,
    spi_read_j,
    spi_transceive_j,
};

const struct periph_api_t spi_periph_api = {
    .init = spi_init,
    .get_dev = spi_get_dev,
    .fns = spi_periph_fns,
    .num_fns = ARRAY_SIZE(spi_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, spi),
    .name = "spi",
};

#else

const struct periph_api_t spi_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "spi",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, spi)
