#include <sys/byteorder.h>
#include <jabi.h>
#include <peripherals/metadata.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_metadata, CONFIG_LOG_DEFAULT_LEVEL);

PERIPH_FUNC_DEF(get_serial) {
    PERIPH_FUNC_GET_RET(metadata, get_serial);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    strncpy((char*) ret, CONFIG_JABI_SERIAL, RESP_PAYLOAD_MAX_SIZE);
    *resp_len = strlen(CONFIG_JABI_SERIAL);

    return 0;
}

PERIPH_FUNC_DEF(get_num_inst) {
    PERIPH_FUNC_GET_ARGS(metadata, get_num_inst);
    PERIPH_FUNC_GET_RET(metadata, get_num_inst);
    PERIPH_FUNC_CHECK_ARGS_LEN(metadata, get_num_inst);

    args->periph_id = sys_le16_to_cpu(args->periph_id);

    LOG_DBG("(periph_id=0x%x)", args->periph_id);

    if (args->periph_id >= NUM_PERIPHERALS) {
        LOG_ERR("bad peripheral id %d", args->periph_id);
        return JABI_NOT_SUPPORTED_ERR;
    }

    extern const struct periph_api_t *peripherals[];
    ret->num_idx = sys_cpu_to_le16(peripherals[args->periph_id]->num_idx);
    *resp_len = sizeof(metadata_get_num_inst_resp_t);

    return 0;
}

static periph_func_t metadata_periph_fns[] = {
    get_serial,
    get_num_inst,
};

const struct periph_api_t metadata_periph_api = {
    .fns = metadata_periph_fns,
    .num_fns = ARRAY_SIZE(metadata_periph_fns),
    .num_idx = 1,
    .name = "metadata",
};
