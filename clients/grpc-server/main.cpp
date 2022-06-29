#include <getopt.h>

#include "server.h"

int main(int argc, char* argv[]) {
    // default args
    std::string ip = "0.0.0.0";
    std::string port = "42069";
    std::string interface = "usb";
    std::string sn = "69420";
    std::string tty = "/dev/tty.usbmodem14402";
    std::string baud = "115200";

    // parse args
    const struct option options[] = {
        { .name = "ip",        .has_arg = 1, .flag = NULL, .val =  0  },
        { .name = "port",      .has_arg = 1, .flag = NULL, .val = 'p' },
        { .name = "interface", .has_arg = 1, .flag = NULL, .val = 'i' },
        { .name = "sn",        .has_arg = 1, .flag = NULL, .val = 's' },
        { .name = "tty",       .has_arg = 1, .flag = NULL, .val = 't' },
        { .name = "baud",      .has_arg = 1, .flag = NULL, .val = 'b' },
        {0,0,0,0}
    };

    int opt, longindex;
    while ((opt = getopt_long(argc, argv, ":p:i:s:t:b:", options, &longindex)) != -1) {
        switch (opt) {
            case 0:
                switch (longindex) {
                    case 0: ip = optarg; break;
                }
                break;
            case 'p': port      = optarg; break;
            case 'i': interface = optarg; break;
            case 's': sn        = optarg; break;
            case 't': tty       = optarg; break;
            case 'b': baud      = optarg; break;
            default:
                std::cerr << "bad args" << std::endl;
                exit(0);
                break;
        }
    }

    // open device
    std::shared_ptr<jabi::Device> dev;
    if (interface == "usb") {
        bool found = false;
        for (auto &d : jabi::USBInterface::list_devices()) {
            if (d.get_serial() == sn) {
                dev = std::make_shared<jabi::Device>(d);
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "couldn't find USB device" << std::endl;
            exit(0);
        }
    } else if (interface == "uart") {
        dev = std::make_shared<jabi::Device>(
            jabi::UARTInterface::get_device(tty, std::stoi(baud)));
    } else {
        std::cerr << "invalid interface" << std::endl;
        exit(0);
    }

    // start server
    ServerBuilder builder;
    builder.AddListeningPort(ip + ":" + port, grpc::InsecureServerCredentials());

    JABIServiceImpl service(dev);
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();

    return 0;
}
