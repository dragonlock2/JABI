#include <iostream>

#include <grpcpp/grpcpp.h>
#include "jabi.grpc.pb.h"

#include <libjabi/interfaces/usb.h>
#include <libjabi/interfaces/uart.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using google::protobuf::Empty;
using google::protobuf::StringValue;
using google::protobuf::UInt32Value;
using google::protobuf::BytesValue;
using google::protobuf::BoolValue;
using google::protobuf::Int32Value;

using namespace JABI;

class JABIServiceImpl final : public Device::Service {
public:
    JABIServiceImpl(std::shared_ptr<jabi::Device> dev) : dev(dev) {}

    /* Metadata */
    Status serial(ServerContext*, const Empty*, StringValue* resp) override;
    Status num_inst(ServerContext*, const NumInstRequest* req, UInt32Value* resp) override;
    Status echo(ServerContext*, const StringValue* req, StringValue* resp) override;
    Status req_max_size(ServerContext*, const Empty*, UInt32Value* resp) override;
    Status resp_max_size(ServerContext*, const Empty*, UInt32Value* resp) override;
    Status custom(ServerContext*, const BytesValue* req, BytesValue* resp) override;

    /* CAN */
    Status can_set_filter(ServerContext*, const CANSetFilterRequest* req, Empty*) override;
    Status can_set_rate(ServerContext*, const CANSetRateRequest* req, Empty*) override;
    Status can_set_mode(ServerContext*, const CANSetModeRequest* req, Empty*) override;
    Status can_state(ServerContext*, const Index* req, CANStateResponse* resp) override;
    Status can_write(ServerContext*, const CANWriteRequest* req, Empty*) override;
    Status can_read(ServerContext*, const Index* req, CANReadResponse* resp) override;

    /* I2C */
    Status i2c_set_freq(ServerContext*, const I2CSetFreqRequest* req, Empty*) override;
    Status i2c_write(ServerContext*, const I2CWriteRequest* req, Empty*) override;
    Status i2c_read(ServerContext*, const I2CReadRequest* req, BytesValue* resp) override;
    Status i2c_transceive(ServerContext*, const I2CTransceiveRequest* req, BytesValue* resp) override;

    /* GPIO */
    Status gpio_set_mode(ServerContext*, const GPIOSetModeRequest* req, Empty*) override;
    Status gpio_write(ServerContext*, const GPIOWriteRequest* req, Empty*) override;
    Status gpio_read(ServerContext*, const Index* req, BoolValue* resp) override;

    /* PWM */
    Status pwm_write(ServerContext*, const PWMWriteRequest* req, Empty*) override;

    /* ADC */
    Status adc_read(ServerContext*, const Index* req, Int32Value* resp) override;

    /* DAC */
    Status dac_write(ServerContext*, const DACWriteRequest* req, Empty*) override;

    /* SPI */
    Status spi_set_freq(ServerContext*, const SPISetFreqRequest* req, Empty*) override;
    Status spi_set_mode(ServerContext*, const SPISetModeRequest* req, Empty*) override;
    Status spi_set_bitorder(ServerContext*, const SPISetBitorderRequest* req, Empty*) override;
    Status spi_write(ServerContext*, const SPIWriteRequest* req, Empty*) override;
    Status spi_read(ServerContext*, const SPIReadRequest* req, BytesValue* resp) override;
    Status spi_transceive(ServerContext*, const SPITransceiveRequest* req, BytesValue* resp) override;

    /* UART */
    Status uart_set_config(ServerContext*, const UARTSetConfigRequest* req, Empty*) override;
    Status uart_write(ServerContext*, const UARTWriteRequest* req, Empty*) override;
    Status uart_read(ServerContext*, const UARTReadRequest* req, BytesValue* resp) override;

    /* LIN */
    Status lin_set_mode(ServerContext*, const LINSetModeRequest* req, Empty*) override;
    Status lin_set_rate(ServerContext*, const LINSetRateRequest* req, Empty*) override;
    Status lin_set_filter(ServerContext*, const LINSetFilterRequest* req, Empty*) override;
    Status lin_mode(ServerContext*, const Index* req, LINModeResponse* resp) override;
    Status lin_status(ServerContext*, const Index* req, LINStatusResponse* resp) override;
    Status lin_write(ServerContext*, const LINWriteRequest* req, Empty*) override;
    Status lin_read(ServerContext*, const LINReadRequest* req, LINReadResponse* resp) override;

private:
    std::shared_ptr<jabi::Device> dev;
};
