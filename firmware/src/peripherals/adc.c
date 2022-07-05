#include <drivers/adc.h>
#include <sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/adc.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_adc, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(jabi_adc)

typedef struct {
    const struct device *dev;
    uint8_t channel;
    uint8_t resolution;
    int32_t vref;
} adc_dev_data_t;

#define GEN_ADC_DEV_DATA(node_id, prop, idx)                               \
    {                                                                      \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(JABI_ADC_NODE, devices, idx)), \
        .channel = DT_PROP_BY_IDX(JABI_ADC_NODE, channels, idx),           \
        .resolution = DT_PROP_BY_IDX(JABI_ADC_NODE, resolutions, idx),     \
        .vref = DT_PROP_BY_IDX(JABI_ADC_NODE, vrefs_mv, idx),              \
    },

static adc_dev_data_t adc_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_ADC_NODE, devices, GEN_ADC_DEV_DATA)
};

static int adc_init(uint16_t idx) {
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(adc_read_j) {
    PERIPH_FUNC_GET_RET(adc, read_j);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    uint16_t samples[1];
    struct adc_channel_cfg cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_MAX,
        .channel_id       = adc_devs[idx].channel,
        .differential     = 0
    };
    struct adc_sequence seq = {
        .options      = NULL,
        .channels     = BIT(adc_devs[idx].channel),
        .buffer       = samples,
        .buffer_size  = sizeof(samples),
        .resolution   = adc_devs[idx].resolution,
        .oversampling = 0,
        .calibrate    = false
    };
    if (adc_channel_setup(adc_devs[idx].dev, &cfg)) {
        LOG_ERR("failed to setup channel");
        return JABI_PERIPHERAL_ERR;
    }
    if (adc_read(adc_devs[idx].dev, &seq)) {
        LOG_ERR("failed to read");
        return JABI_PERIPHERAL_ERR;
    }
    int32_t voltage = samples[0];
    if (adc_raw_to_millivolts(adc_devs[idx].vref, ADC_GAIN_1,
            adc_devs[idx].resolution, &voltage)) {
        LOG_ERR("failed to convert?!");
        return JABI_PERIPHERAL_ERR;
    }
    ret->mv = voltage;
    *resp_len = sizeof(adc_read_j_resp_t);
    return JABI_NO_ERR;
}

static const periph_func_t adc_periph_fns[] = {
    adc_read_j,
};

const struct periph_api_t adc_periph_api = {
    .init = adc_init,
    .fns = adc_periph_fns,
    .num_fns = ARRAY_SIZE(adc_periph_fns),
    .num_idx = ARRAY_SIZE(adc_devs),
    .name = "adc",
};

#else

const struct periph_api_t adc_periph_api = {
    .init = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "adc",
};

#endif // DT_HAS_COMPAT_STATUS_OKAY(jabi_adc)
