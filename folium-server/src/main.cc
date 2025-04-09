#include <atomic>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

#include "auth.h"
#include "logger.h"
#include "version.h"
#include "fifo_util.h"
#include "dispatcher.h"
#include "http_gateway.h"

std::string ip = "127.0.0.1";
int port = 50105;
unsigned int num_threads = 10;

const std::string GW2DP = "GW2DP";
const std::string DP2GW = "DP2GW";

int main(void)
{
    Logger::log("Starting Folium Server v" + Folium::VERSION);
    Logger::logS("Folium Server v", Folium::VERSION, " (build ", Folium::BUILD_ID, " ", Folium::BUILD_DATE, ")");

    // Auto-cleanup on crash or Ctrl+C
    ipc::install_signal_handler();
    ipc::ScopedFifo fifoIn(GW2DP);
    ipc::ScopedFifo fifoOut(DP2GW);

    /*
        start gateway on parent process and
        start dispatch on child process
    */
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0)
    {
        // child
        Logger::logS("Dispatch process online with pid: ", pid);

        Dispatcher::create_threads(10);

        Logger::log("Dispatch process done, closing...");

        return 0;
    }
    else
    {
        // parent
        Logger::logS("Gateway process online with pid: ", pid);

        // create gateway
        gateway::Gateway gateway;
        gateway.listen(ip, port);

        // listen for input (to close)
        std::cout << "Type 'exit' to stop the server" << std::endl;
        std::string input;
        while (true) {
            std::getline(std::cin, input);

            if (input == "exit")
            {
                std::cout << "Exit command received. Shutting down..." << std::endl;
                gateway.stop();
                break;
            }
        }

        // after close, print and wait for child process to finish
        Logger::log("Gateway process done, merging with Gateway process...");
        wait(nullptr);

        Logger::log("Processes merged.");
    }

    Logger::log("Folium Server Closed.");
    return 0;
}
