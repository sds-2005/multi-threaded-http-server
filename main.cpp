#include "TcpServer.h"
#include <cstdlib>

int main(int argc, char* argv[]) {
    int threads = 4;
    if (argc > 1) {
        threads = atoi(argv[1]);
        if (threads < 1) threads = 1;
    }
    TcpServer server("0.0.0.0", 8080, threads);
    server.startListen();
    return 0;
}
