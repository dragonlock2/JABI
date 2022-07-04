#include <thread>
#include <chrono>
#include <getopt.h>

#include "client.h"

/* Copied from tests/libjabi/main.cpp ✨ */

using namespace std::chrono_literals;

void testDevice(jabi::gRPCDevice &d) {
    /* Metadata */
    std::cout << "SN=" << d.serial();
    std::cout << " num_meta=" << d.num_inst(jabi::InstID::METADATA);
    std::cout << " echo=" << d.echo("❤️");
    std::cout << std::endl;

    /* CAN */
    auto lim = d.num_inst(jabi::InstID::CAN);
    for (auto i = 0; i < lim; i++) {
        std::cout << "\tListening only to 0x69420 messages on CAN " << i << std::endl;
        d.can_set_rate(125000, 1000000, i);
        d.can_set_filter(0x69420, 0xFFFFF, 0, 0, i);
        d.can_set_mode(jabi::CANMode::NORMAL, i);
        auto s = d.can_state(i);
        std::cout << "\tstate: " << s.state << " tx_err: " << s.tx_err;
        std::cout << " rx_err: " << s.rx_err << std::endl;

        d.can_write(jabi::CANMessage(0x69420, std::vector<uint8_t>{69, 42}), i);
        d.can_write(jabi::CANMessage(0x69420, 2), i);
        std::cout << "\tSent some messages" << std::endl;

        std::this_thread::sleep_for(500ms);
        std::cout << "\tPrinting received messsages" << std::endl;
        jabi::CANMessage msg;

        // less LOC, but one extra check
        while (d.can_read(msg, i) != -1) {
            std::cout << "\t " << msg << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    //default args
    std::string ip = "localhost";
    std::string port = "42069";

    // parse args
    const struct option options[] = {
        { .name = "ip",   .has_arg = 1, .flag = NULL, .val = 'i' },
        { .name = "port", .has_arg = 1, .flag = NULL, .val = 'p' },
        {0,0,0,0}
    };

    int opt, longindex;
    while ((opt = getopt_long(argc, argv, ":i:p:", options, &longindex)) != -1) {
        switch (opt) {
            case 'i': ip   = optarg; break;
            case 'p': port = optarg; break;
            default:
                std::cerr << "bad args" << std::endl;
                exit(0);
                break;
        }
    }

    // connect client
    auto channel = grpc::CreateChannel(ip + ":" + port, grpc::InsecureChannelCredentials());
    jabi::gRPCDevice dev(channel);
    testDevice(dev);
    return 0;
}
