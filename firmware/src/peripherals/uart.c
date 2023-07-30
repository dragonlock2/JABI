#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/byteorder.h>
#include <jabi.h>
#include <jabi/peripherals/uart.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(periph_uart, CONFIG_LOG_DEFAULT_LEVEL);

#if DT_NODE_HAS_PROP(JABI_PERIPH_NODE, uart)

#define TX_TIMEOUT K_MSEC(100)
#define RX_TIMEOUT K_MSEC(100) // time between chars

typedef struct {
    const struct device *dev;
    struct k_sem tx_lock;
    bool tx_was_busy;
    uint8_t tx_buf[REQ_PAYLOAD_MAX_SIZE];
    size_t tx_len;
    uint8_t *tx_ptr;
    struct k_msgq *rx_msgq;
} uart_dev_data_t;

#define GEN_UART_DEV_DATA(node_id, prop, idx)                     \
    {                                                             \
        .dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)), \
        .tx_was_busy = false,                                     \
        .tx_len = 0,                                              \
        .rx_msgq = &uart_msgq##idx,                               \
    },

#define GEN_UART_MSGQ(node_id, prop, idx) \
    K_MSGQ_DEFINE(uart_msgq##idx, 1, CONFIG_JABI_UART_RX_BUFFER_SIZE, 1);

DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, uart, GEN_UART_MSGQ);

static uart_dev_data_t uart_devs[] = {
    DT_FOREACH_PROP_ELEM(JABI_PERIPH_NODE, uart, GEN_UART_DEV_DATA)
};

static void uart_handler(const struct device *dev, void *data) {
    uart_dev_data_t *dev_data = data;
    while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
        if (uart_irq_tx_ready(dev)) {
            if (dev_data->tx_len == 0) {
                uart_irq_tx_disable(dev);
                k_sem_give(&dev_data->tx_lock);
                continue;
            }
            int len = uart_fifo_fill(dev, dev_data->tx_ptr, dev_data->tx_len);
            if (len < 0) {
                LOG_ERR("failed to write uart?!");
                continue;
            }
            dev_data->tx_len -= len;
            dev_data->tx_ptr += len;
        }

        if (uart_irq_rx_ready(dev)) {
            uint8_t buffer[64];
            int len = uart_fifo_read(dev, buffer, sizeof(buffer));
            if (len < 0) {
                LOG_ERR("failed to read uart?!");
                continue;
            }
            for (int i = 0; i < len; i++) {
                if (k_msgq_put(dev_data->rx_msgq, &buffer[i], K_NO_WAIT)) {
                    LOG_ERR("buffer full, purging all of it");
                    k_msgq_purge(dev_data->rx_msgq);
                    break;
                }
            }
        }
    }
}

static int uart_init(uint16_t idx) {
    k_sem_init(&uart_devs[idx].tx_lock, 0, 1);
    uart_irq_callback_user_data_set(uart_devs[idx].dev, uart_handler, &uart_devs[idx]);
    uart_irq_rx_enable(uart_devs[idx].dev);
    return JABI_NO_ERR;
}

static void *uart_get_dev(uint16_t idx) {
    return (void*) uart_devs[idx].dev;
}

PERIPH_FUNC_DEF(uart_set_config) {
    PERIPH_FUNC_GET_ARGS(uart, set_config);
    PERIPH_FUNC_CHECK_ARGS_LEN(uart, set_config);
    args->baud = sys_le32_to_cpu(args->baud);

    uart_set_config_req_t *tmp = args;
    LOG_DBG("(baud=%d,data_bits=%d,parity=%d,stop_bits=%d)",
        tmp->baud, tmp->data_bits, tmp->parity, tmp->stop_bits);

    struct uart_config cfg;
    cfg.baudrate = args->baud;
    switch (args->data_bits) {
        case 5: cfg.data_bits = UART_CFG_DATA_BITS_5; break;
        case 6: cfg.data_bits = UART_CFG_DATA_BITS_6; break;
        case 7: cfg.data_bits = UART_CFG_DATA_BITS_7; break;
        case 8: cfg.data_bits = UART_CFG_DATA_BITS_8; break;
        case 9: cfg.data_bits = UART_CFG_DATA_BITS_9; break;
        default:
            LOG_ERR("invalid number of data bits %d", args->data_bits);
            return JABI_INVALID_ARGS_ERR;
    }
    switch (args->parity) {
        case 0: cfg.parity = UART_CFG_PARITY_NONE;  break;
        case 1: cfg.parity = UART_CFG_PARITY_ODD;   break;
        case 2: cfg.parity = UART_CFG_PARITY_EVEN;  break;
        case 3: cfg.parity = UART_CFG_PARITY_MARK;  break;
        case 4: cfg.parity = UART_CFG_PARITY_SPACE; break;
        default:
            LOG_ERR("invalid parity %d", args->parity);
            return JABI_INVALID_ARGS_ERR;
    }
    switch (args->stop_bits) {
        case 0: cfg.stop_bits = UART_CFG_STOP_BITS_0_5; break;
        case 1: cfg.stop_bits = UART_CFG_STOP_BITS_1;   break;
        case 2: cfg.stop_bits = UART_CFG_STOP_BITS_1_5; break;
        case 3: cfg.stop_bits = UART_CFG_STOP_BITS_2;   break;
        default:
            LOG_ERR("invalid number of stop bits %d", args->stop_bits);
            return JABI_INVALID_ARGS_ERR;
    }
    cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;

    if (uart_configure(uart_devs[idx].dev, &cfg)) {
        LOG_ERR("failed to configure");
        return JABI_PERIPHERAL_ERR;
    }
    *resp_len = 0;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(uart_write) {
    PERIPH_FUNC_GET_ARGS(uart, write);

    LOG_DBG("()");
    LOG_HEXDUMP_DBG(args, req_len, "data=");

    *resp_len = 0;
    if (uart_devs[idx].tx_was_busy) {
        if (k_sem_take(&uart_devs[idx].tx_lock, K_NO_WAIT)) {
            return JABI_BUSY_ERR;
        }
    }
    memcpy(uart_devs[idx].tx_buf, args, req_len);
    uart_devs[idx].tx_len = req_len;
    uart_devs[idx].tx_ptr = uart_devs[idx].tx_buf;
    uart_irq_tx_enable(uart_devs[idx].dev);
    if (k_sem_take(&uart_devs[idx].tx_lock, TX_TIMEOUT)) {
        uart_devs[idx].tx_was_busy = true;
        return JABI_BUSY_ERR;
    }
    uart_devs[idx].tx_was_busy = false;
    return JABI_NO_ERR;
}

PERIPH_FUNC_DEF(uart_read) {
    PERIPH_FUNC_GET_ARGS(uart, read);
    PERIPH_FUNC_GET_RET(uart, read);
    PERIPH_FUNC_CHECK_ARGS_LEN(uart, read);
    args->data_len = sys_le16_to_cpu(args->data_len);

    if (args->data_len > RESP_PAYLOAD_MAX_SIZE) {
        LOG_ERR("requested read too long");
        return JABI_INVALID_ARGS_ERR;
    }

    LOG_DBG("(data_len=%d)", args->data_len);

    *resp_len = 0;
    for (int i = 0; i < args->data_len; i++) {
        if (k_msgq_get(uart_devs[idx].rx_msgq, &ret[i], RX_TIMEOUT)) {
            break;
        }
        *resp_len += 1;
    }
    return JABI_NO_ERR;
}

static const periph_func_t uart_periph_fns[] = {
    uart_set_config,
    uart_write,
    uart_read,
};

const struct periph_api_t uart_periph_api = {
    .init = uart_init,
    .get_dev = uart_get_dev,
    .fns = uart_periph_fns,
    .num_fns = ARRAY_SIZE(uart_periph_fns),
    .num_idx = DT_PROP_LEN(JABI_PERIPH_NODE, uart),
    .name = "uart",
};

#else

const struct periph_api_t uart_periph_api = {
    .init = NULL,
    .get_dev = NULL,
    .fns = NULL,
    .num_fns = 0,
    .num_idx = 0,
    .name = "uart",
};

#endif // DT_NODE_HAS_PROP(JABI_PERIPH_NODE, uart)
