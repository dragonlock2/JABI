#include <drivers/pwm.h>
#include <sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/pwm.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_pwm, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, pwm)

static const struct pwm_dt_spec pwms[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, pwm, ELEM_TO_PWM)
};

static int pwm_init(uint16_t idx) {
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(pwm_write) {
    PERIPH_FUNC_GET_ARGS(pwm, write);
    PERIPH_FUNC_CHECK_ARGS_LEN(pwm, write);
    args->pulsewidth = sys_le32_to_cpu(args->pulsewidth);
    args->period = sys_le32_to_cpu(args->period);

    pwm_write_req_t *tmp = args;
    LOG_DBG("(pulsewidth=%u,period=%u)", tmp->pulsewidth, tmp->period);

    if (pwm_set(pwms[idx].dev, pwms[idx].channel, args->period, args->pulsewidth, 0)) {
        LOG_ERR("failed to set PWM");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

static const periph_func_t pwm_periph_fns[] = {
    pwm_write,
};

const struct periph_api_t pwm_periph_api = {
    .init = pwm_init,
    .fns = pwm_periph_fns,
    .num_fns = ARRAY_SIZE(pwm_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, pwm),
    .name = "pwm",
};

#else

const struct periph_api_t pwm_periph_api = {
    .init = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "pwm",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, pwm)
