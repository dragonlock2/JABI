#include <getopt.h>

#include "client.h"

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
    JABIClient dev(
        grpc::CreateChannel(ip + ":" + port, grpc::InsecureChannelCredentials())
    );

    std::cout << "Attached to server: ";
    std::cout << "SN=" << dev.serial();
    std::cout << " num_meta=" << dev.num_inst(jabi::METADATA_ID);
    std::cout << " echo=" << dev.echo("❤️");
    std::cout << std::endl;
    return 0;
}
