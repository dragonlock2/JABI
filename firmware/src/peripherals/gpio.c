#include <drivers/gpio.h>
#include <jabi.h>
#include <jabi/peripherals/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_gpio, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, gpio)

static const struct gpio_dt_spec gpios[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, gpio, ELEM_TO_GPIO)
};

static int gpio_init(uint16_t idx) {
    return JABI_NO_ERR;
}

static void *gpio_get_dev(uint16_t idx) {
    return (void*) gpios[idx].port;
}

PERIPH_FUNC_DEF(gpio_set_mode) {
    PERIPH_FUNC_GET_ARGS(gpio, set_mode);
    PERIPH_FUNC_CHECK_ARGS_LEN(gpio, set_mode);

    gpio_set_mode_req_t *tmp = args;
    LOG_DBG("(direction=%d,pull=%d,init_val=%d)",
        tmp->direction, tmp->pull, tmp->init_val);

    gpio_flags_t flags = GPIO_ACTIVE_HIGH;
    switch (args->direction) {
        case 0: flags |= GPIO_INPUT;       break;
        case 1: flags |= GPIO_OUTPUT;      break;
        case 2: flags |= GPIO_OPEN_DRAIN;  break;
        case 3: flags |= GPIO_OPEN_SOURCE; break;
        default:
            LOG_ERR("invalid direction");
            return JABI_INVALID_ARGS_ERR;
    }
    switch (args->pull) {
        case 0:                                         break;
        case 1: flags |= GPIO_PULL_UP;                  break;
        case 2: flags |= GPIO_PULL_DOWN;                break;
        case 3: flags |= GPIO_PULL_UP | GPIO_PULL_DOWN; break;
        default:
            LOG_ERR("invalid pull");
            return JABI_INVALID_ARGS_ERR;
    }
    if (args->direction) { // output
        flags |= args->init_val ? GPIO_OUTPUT_HIGH : GPIO_OUTPUT_LOW;
    }
    if (gpio_pin_configure(gpios[idx].port, gpios[idx].pin, flags)) {
        LOG_ERR("failed to configure gpio");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(gpio_write) {
    PERIPH_FUNC_GET_ARGS(gpio, write);
    PERIPH_FUNC_CHECK_ARGS_LEN(gpio, write);

    LOG_DBG("(val=%d)", args->val);

    if (gpio_pin_set_dt(&gpios[idx], args->val)) {
        LOG_ERR("failed to write pin");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(gpio_read) {
    PERIPH_FUNC_GET_RET(gpio, read);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    int r = gpio_pin_get_dt(&gpios[idx]);
    if (r < 0) {
        LOG_ERR("failed to read pin");
        return JABI_PERIPHERAL_ERR;
    }
    ret->val = r;
    *resp_len = sizeof(gpio_read_resp_t);
    return JABI_NO_ERR;
}

static const periph_func_t gpio_periph_fns[] = {
    gpio_set_mode,
    gpio_write,
    gpio_read,
};

const struct periph_api_t gpio_periph_api = {
    .init = gpio_init,
    .get_dev = gpio_get_dev,
    .fns = gpio_periph_fns,
    .num_fns = ARRAY_SIZE(gpio_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, gpio),
    .name = "gpio",
};

#else

const struct periph_api_t gpio_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "gpio",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, gpio)
