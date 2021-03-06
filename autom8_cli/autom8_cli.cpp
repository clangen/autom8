#include "stdafx.h"

#include <autom8/autom8.h>
#include <boost/thread.hpp>

#ifdef WIN32
#include <Windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#include <signal.h>
#endif

static void rpc_recv_handler(const char* json) {
    std::cerr << "recv: " << json << std::endl;
}

int main(int argc, char* argv[])
{
    autom8_init();
    autom8_set_rpc_callback(rpc_recv_handler);
    printf("library version: %s\n\n", autom8_version());
    for (int i = 0; i < 100; i++) {
        autom8_rpc("{\"component\": \"server\", \"command\": \"set_preference\", \"options\": { \"key\": \"password\", \"value\": \"c66247c6abc4c8d745969b4ea11674c7ad8579a40ce1e5da01ff3a845dc72d35\" } }");
    }
    // autom8_rpc("{\"component\": \"server\", \"command\": \"get_preference\", \"options\": { \"key\": \"password\" } }", 0);
    autom8_rpc("{\"component\": \"server\", \"command\": \"start\"}");
    // autom8_rpc("{\"component\": \"system\", \"command\": \"list\"}", 0);
    // autom8_rpc("{\"component\": \"system\", \"command\": \"current\"}", 0);
    // autom8_rpc("{\"component\": \"system\", \"command\": \"add_device\", \"options\": { \"label\": \"office\", \"type\": 0, \"address\": \"a1\" } }", 0);
    // autom8_rpc("{\"component\": \"system\", \"command\": \"list_devices\" }", 0);
    // sleep(60);
    // autom8_rpc("{\"component\": \"server\", \"command\": \"stop\"}", 0);
    // autom8_deinit();
    // printf("\nautom8-cli finished...\n");
    // getchar();

    while(true) { sleep(60); }

    return 0;
}
