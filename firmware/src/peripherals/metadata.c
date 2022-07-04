#include <sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/metadata.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(periph_metadata, CONFIG_LOG_DEFAULT_LEVEL);

static int metadata_init(uint16_t idx) {
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(serial) {
    PERIPH_FUNC_GET_RET(metadata, serial);
    PERIPH_FUNC_CHECK_ARGS_EMPTY;

    LOG_DBG("()");

    strncpy((char*) ret, CONFIG_JABI_SERIAL, RESP_PAYLOAD_MAX_SIZE);
    *resp_len = strlen(CONFIG_JABI_SERIAL);

    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(num_inst) {
    PERIPH_FUNC_GET_ARGS(metadata, num_inst);
    PERIPH_FUNC_GET_RET(metadata, num_inst);
    PERIPH_FUNC_CHECK_ARGS_LEN(metadata, num_inst);

    args->periph_id = sys_le16_to_cpu(args->periph_id);

    LOG_DBG("(periph_id=0x%x)", args->periph_id);

    if (args->periph_id >= NUM_PERIPHERALS) {
        LOG_ERR("bad peripheral id %d", args->periph_id);
        return JABI_NOT_SUPPORTED_ERR;
    }

    extern const struct periph_api_t *peripherals[];
    ret->num_idx = sys_cpu_to_le16(peripherals[args->periph_id]->num_idx);
    *resp_len = sizeof(metadata_num_inst_resp_t);

    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(echo) {
    PERIPH_FUNC_GET_ARGS(metadata, echo);
    PERIPH_FUNC_GET_RET(metadata, echo);

    LOG_DBG("(len=%d)", req_len);

    if (req_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("input string too long");
        return JABI_INVALID_ARGS_ERR;
    }

    memcpy(ret, args, req_len);
    *resp_len = req_len;

    return JABI_NO_ERR;
}

static const periph_func_t metadata_periph_fns[] = {
    serial,
    num_inst,
    echo,
};

const struct periph_api_t metadata_periph_api = {
    .init = metadata_init,
    .fns = metadata_periph_fns,
    .num_fns = ARRAY_SIZE(metadata_periph_fns),
    .num_idx = 1,
    .name = "metadata",
};
