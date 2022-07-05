#include <jabi.h>

extern const struct periph_api_t metadata_periph_api;
extern const struct periph_api_t can_periph_api;
extern const struct periph_api_t i2c_periph_api;
extern const struct periph_api_t gpio_periph_api;
extern const struct periph_api_t pwm_periph_api;
extern const struct periph_api_t adc_periph_api;
extern const struct periph_api_t dac_periph_api;

const struct periph_api_t *peripherals[] = {
    &metadata_periph_api,
    &can_periph_api,
    &i2c_periph_api,
    &gpio_periph_api,
    &pwm_periph_api,
    &adc_periph_api,
    &dac_periph_api,
};
