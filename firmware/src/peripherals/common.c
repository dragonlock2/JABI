#include <jabi.h>

extern const struct periph_api_t metadata_periph_api;
extern const struct periph_api_t can_periph_api;

const struct periph_api_t *peripherals[] = {
    &metadata_periph_api,
    &can_periph_api,
};
