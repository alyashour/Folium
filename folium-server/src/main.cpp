#include "dispatcher.hpp"
#include "gateway.hpp"
#include "logger.hpp"

#include <string>

using namespace std;

string version = "0.1.0";
string ip = "127.0.0.1";
string port = "90105";
unsigned int num_threads = 1000;

int main(void) {
  Logger::log("Starting Folium Server v" + version);

  bool success = Dispatcher::create_threads(num_threads);
  if (!success) {
    Logger::log_err("Failed to start threads, closing program...");
    exit(1);
  }

  Gateway::listen(ip, port);
}