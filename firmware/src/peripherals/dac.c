#include <zephyr/drivers/dac.h>
#include <zephyr/sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/dac.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(periph_dac, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(jabi_dac)

typedef struct {
    const struct device *dev;
    uint8_t channel;
    uint8_t resolution;
    int32_t vref;
} dac_dev_data_t;

#define GEN_DAC_DEV_DATA(node_id, prop, idx)                               \
    {                                                                      \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(JABI_DAC_NODE, devices, idx)), \
        .channel = DT_PROP_BY_IDX(JABI_DAC_NODE, channels, idx),           \
        .resolution = DT_PROP_BY_IDX(JABI_DAC_NODE, resolutions, idx),     \
        .vref = DT_PROP_BY_IDX(JABI_DAC_NODE, vrefs_mv, idx),              \
    },

static dac_dev_data_t dac_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_DAC_NODE, devices, GEN_DAC_DEV_DATA)
};

static int dac_init(uint16_t idx) {
    return JABI_NO_ERR;
}

static void *dac_get_dev(uint16_t idx) {
    return (void*) dac_devs[idx].dev;
}

PERIPH_FUNC_DEF(dac_write) {
    PERIPH_FUNC_GET_ARGS(dac, write);
    PERIPH_FUNC_CHECK_ARGS_LEN(dac, write);
    args->mv = sys_le32_to_cpu(args->mv);

    LOG_DBG("(mv=%d)", args->mv);

    struct dac_channel_cfg cfg = {
        .channel_id = dac_devs[idx].channel,
        .resolution = dac_devs[idx].resolution,
    };
    if (dac_channel_setup(dac_devs[idx].dev, &cfg)) {
        LOG_ERR("failed to setup channel");
        return JABI_PERIPHERAL_ERR;
    }
    if (dac_write_value(dac_devs[idx].dev, dac_devs[idx].channel,
            ((1 << dac_devs[idx].resolution)-1) * args->mv / dac_devs[idx].vref)) {
        LOG_ERR("failed to write channel");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

static const periph_func_t dac_periph_fns[] = {
    dac_write,
};

const struct periph_api_t dac_periph_api = {
    .init = dac_init,
    .get_dev = dac_get_dev,
    .fns = dac_periph_fns,
    .num_fns = ARRAY_SIZE(dac_periph_fns),
    .num_idx = ARRAY_SIZE(dac_devs),
    .name = "dac",
};

#else

const struct periph_api_t dac_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "dac",
};

#endif // DT_HAS_COMPAT_STATUS_OKAY(jabi_dac)
