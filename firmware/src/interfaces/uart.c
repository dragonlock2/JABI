#include <jabi.h>
#include <drivers/uart.h>
#include <sys/byteorder.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(iface_uart, CONFIG_LOG_DEFAULT_LEVEL);

#define RX_TIMEOUT Z_TIMEOUT_MS(1000)

static int read(struct k_msgq *msgq, uint8_t *buf, size_t len, k_timeout_t time) {
    for (int i = 0; i < len; i++) {
        if (k_msgq_get(msgq, &buf[i], time)) {
            return -1;
        }
    }
    return 0;
}

#define CREATE_UART_API(node_id, prop, idx)                                           \
    K_MSGQ_DEFINE(uart##idx##rx, 1, sizeof(iface_req_t), 1);                          \
                                                                                      \
    K_SEM_DEFINE(uart##idx##tx_lock, 0, 1);                                           \
    size_t uart##idx##tx_len;                                                         \
    uint8_t *uart##idx##tx_buf;                                                       \
                                                                                      \
    static void uart##idx##_handler(const struct device *dev, void *data) {           \
        ARG_UNUSED(data);                                                             \
        while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {                    \
            if (uart_irq_rx_ready(dev)) {                                             \
                uint8_t buffer[64];                                                   \
                int len = uart_fifo_read(dev, buffer, sizeof(buffer));                \
                if (len < 0) {                                                        \
                    LOG_ERR("failed to read UART" #idx "?!");                         \
                    continue;                                                         \
                }                                                                     \
                for (int i = 0; i < len; i++) {                                       \
                    if (k_msgq_put(&uart##idx##rx, &buffer[i], K_NO_WAIT)) {          \
                        LOG_ERR("UART" #idx " buffer full! purging queue...");        \
                        k_msgq_purge(&uart##idx##rx);                                 \
                        break;                                                        \
                    }                                                                 \
                }                                                                     \
            }                                                                         \
                                                                                      \
            if (uart_irq_tx_ready(dev)) {                                             \
                if (!uart##idx##tx_len) {                                             \
                    uart_irq_tx_disable(dev);                                         \
                    k_sem_give(&uart##idx##tx_lock);                                  \
                    continue;                                                         \
                }                                                                     \
                int len = uart_fifo_fill(dev, uart##idx##tx_buf, uart##idx##tx_len);  \
                uart##idx##tx_len -= len;                                             \
                uart##idx##tx_buf += len;                                             \
            }                                                                         \
        }                                                                             \
    }                                                                                 \
                                                                                      \
    static void uart##idx##_init() {                                                  \
        const struct device *dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)); \
        uart_irq_callback_set(dev, uart##idx##_handler);                              \
        uart_irq_rx_enable(dev);                                                      \
    }                                                                                 \
                                                                                      \
    static void uart##idx##_get_req(iface_req_t *req) {                               \
        while (1) {                                                                   \
            read(&uart##idx##rx, (uint8_t*) req,                                      \
                 sizeof(req->periph_id) +                                             \
                 sizeof(req->periph_idx) +                                            \
                 sizeof(req->payload_len), K_FOREVER);                                \
            req->payload_len = sys_le16_to_cpu(req->payload_len);                     \
            if (req->payload_len > REQ_PAYLOAD_MAX_SIZE) {                            \
                LOG_ERR("UART" #idx " bad req payload length %d", req->payload_len);  \
                k_msgq_purge(&uart##idx##rx);                                         \
                continue;                                                             \
            }                                                                         \
            if (read(&uart##idx##rx, req->payload, req->payload_len, RX_TIMEOUT)) {   \
                LOG_ERR("UART" #idx " timeout waiting for payload");                  \
                continue;                                                             \
            }                                                                         \
            break;                                                                    \
        }                                                                             \
    }                                                                                 \
                                                                                      \
    static void uart##idx##_send_resp(iface_resp_t *resp) {                           \
        const struct device *dev = DEVICE_DT_GET(DT_PROP_BY_IDX(node_id, prop, idx)); \
        if (resp->payload_len > RESP_PAYLOAD_MAX_SIZE) {                              \
            LOG_ERR("UART" #idx " bad resp payload length %d", resp->payload_len);    \
            return;                                                                   \
        }                                                                             \
        uart##idx##tx_len = sizeof(resp->retcode) +                                   \
                            sizeof(resp->payload_len) +                               \
                            resp->payload_len;                                        \
        uart##idx##tx_buf = (uint8_t*) resp;                                          \
        resp->retcode = sys_cpu_to_le16(resp->retcode);                               \
        resp->payload_len = sys_cpu_to_le16(resp->payload_len);                       \
        uart_irq_tx_enable(dev);                                                      \
        k_sem_take(&uart##idx##tx_lock, K_FOREVER);                                   \
    }                                                                                 \
                                                                                      \
    const struct iface_api_t uart##idx##_iface_api = {                                \
        .init = uart##idx##_init,                                                     \
        .get_req = uart##idx##_get_req,                                               \
        .send_resp = uart##idx##_send_resp,                                           \
        .name = "UART" #idx                                                           \
    };

DT_FOREACH_PROP_ELEM(JABI_IFACE_NODE, uart, CREATE_UART_API)
