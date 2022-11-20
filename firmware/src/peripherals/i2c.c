#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/i2c.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(periph_i2c, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, i2c)

static const struct device *i2c_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, i2c, ELEM_TO_DEVICE)
};

static int i2c_init(uint16_t idx) {
    return JABI_NO_ERR;
}

static void *i2c_get_dev(uint16_t idx) {
    return (void*) i2c_devs[idx];
}

PERIPH_FUNC_DEF(i2c_set_freq) {
    PERIPH_FUNC_GET_ARGS(i2c, set_freq);
    PERIPH_FUNC_CHECK_ARGS_LEN(i2c, set_freq);

    i2c_set_freq_req_t *tmp = args;
    LOG_DBG("(preset=%d)", tmp->preset);

    int speed;
    switch (args->preset) {
        case 0: speed = I2C_SPEED_STANDARD;  break;
        case 1: speed = I2C_SPEED_FAST;      break;
        case 2: speed = I2C_SPEED_FAST_PLUS; break;
        case 3: speed = I2C_SPEED_HIGH;      break;
        case 4: speed = I2C_SPEED_ULTRA;     break;
        default:
            LOG_ERR("invalid speed preset");
            return JABI_INVALID_ARGS_ERR;
    }
    if (i2c_configure(i2c_devs[idx], I2C_MODE_MASTER | I2C_SPEED_SET(speed))) {
        LOG_ERR("failed to set config");
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(i2c_write_j) {
    PERIPH_FUNC_GET_ARGS(i2c, write_j);
    args->addr = sys_le16_to_cpu(args->addr);

    if (req_len < sizeof(i2c_write_j_req_t)) {
        LOG_ERR("invalid amount of data provided");
        return JABI_INVALID_ARGS_FORMAT_ERR;
    }
    uint32_t data_len = req_len - sizeof(i2c_write_j_req_t);

    LOG_DBG("(addr=0x%x)", args->addr);
    LOG_HEXDUMP_DBG(args->data, data_len, "data=");

    if (i2c_write(i2c_devs[idx], args->data, data_len, args->addr)) {
        LOG_ERR("failed to write data");
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(i2c_read_j) {
    PERIPH_FUNC_GET_ARGS(i2c, read_j);
    PERIPH_FUNC_GET_RET(i2c, read_j);
    PERIPH_FUNC_CHECK_ARGS_LEN(i2c, read_j);
    args->addr = sys_le16_to_cpu(args->addr);
    args->data_len = sys_le16_to_cpu(args->data_len);

    if (args->data_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("requested read too long");
        return JABI_INVALID_ARGS_ERR;
    }

    LOG_DBG("(addr=0x%x,data_len=%d)", args->addr, args->data_len);

    if (i2c_read(i2c_devs[idx], ret, args->data_len, args->addr)) {
        LOG_ERR("failed to read data");
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = args->data_len;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(i2c_transceive) {
    PERIPH_FUNC_GET_ARGS(i2c, transceive);
    PERIPH_FUNC_GET_RET(i2c, transceive);
    args->addr = sys_le16_to_cpu(args->addr);
    args->data_len = sys_le16_to_cpu(args->data_len);

    if (req_len < sizeof(i2c_transceive_req_t)) {
        LOG_ERR("invalid amount of data provided");
        return JABI_INVALID_ARGS_FORMAT_ERR;
    }
    uint32_t data_len = req_len - sizeof(i2c_transceive_req_t);

    if (args->data_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("requested read too long");
        return JABI_INVALID_ARGS_ERR;
    }

    LOG_DBG("(addr=0x%x,data_len=%d)", args->addr, args->data_len);
    LOG_HEXDUMP_DBG(args->data, data_len, "data=");

    if (i2c_write_read(i2c_devs[idx], args->addr, args->data, data_len, ret, args->data_len)) {
        LOG_ERR("failed to read data");
        return JABI_PERIPHERAL_ERR;
    }

    *resp_len = args->data_len;
    return JABI_NO_ERR;
}

static const periph_func_t i2c_periph_fns[] = {
    i2c_set_freq,
    i2c_write_j,
    i2c_read_j,
    i2c_transceive,
};

const struct periph_api_t i2c_periph_api = {
    .init = i2c_init,
    .get_dev = i2c_get_dev,
    .fns = i2c_periph_fns,
    .num_fns = ARRAY_SIZE(i2c_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, i2c),
    .name = "i2c",
};

#else

const struct periph_api_t i2c_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "i2c",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, i2c)
