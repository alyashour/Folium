#include <atomic>
#include <thread>

#include "auth.h"
#include "http_gateway.h"
#include "logger.h"
#include "version.h"

std::string ip = "127.0.0.1";
int port = 50105;
unsigned int num_threads = 10;

int main(void)
{
    Logger::log("Starting Folium Server v" + Folium::VERSION);
    Logger::logS("Folium Server v", Folium::VERSION, " (build ", Folium::BUILD_ID, " ", Folium::BUILD_DATE, ")");

    gateway::Gateway gateway;
    gateway.listen(ip, port);

    // listen for input
    std::cout << "Type 'exit' to stop the server" << std::endl;
    std::string input;
    while (true) {
        std::getline(std::cin, input);

        if (input == "exit") {
            std::cout << "Exit command received. Shutting down..." << std::endl;
            gateway.stop();
            break;
        }
    }

    Logger::log("Folium Server Closed.");
    return 0;
}
