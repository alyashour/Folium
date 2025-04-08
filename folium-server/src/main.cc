#include <atomic>
#include <thread>

#include "auth.h"
#include "http_gateway.h"
#include "logger.h"
#include "version.h"

using namespace std;

string ip = "127.0.0.1";
int port = 50105;
unsigned int num_threads = 10;

std::atomic<bool> running = true;

int main(void)
{
    Logger::log("Starting Folium Server v" + Folium::VERSION);
    Logger::logS("Folium Server v", Folium::VERSION, " (build ", Folium::BUILD_ID, " ", Folium::BUILD_DATE, ")");

    // start the server in a new thread
    thread serverThread([&]() {
        Gateway::listen(ip, port);  // This blocks
        running = false;            // Set to false if it exits unexpectedly
    });

    // listen for input
    std::string input;
    while (running) {
        getline(cin, input);

        if (input == "exit") {
            std::cout << "Exit command received. Shutting down server..." << std::endl;
            Gateway::stop();
            break;
        }
    }

    if (serverThread.joinable()) {
        serverThread.join();
    }

    Logger::log("Done! Closing Folium Server.");
    return 0;
}
