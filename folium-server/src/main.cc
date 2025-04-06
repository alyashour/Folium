// #include "dispatcher.h"
// #include "gateway.h"
#include "logger.h"

#include <string>

using namespace std;

string version = "0.1.0";
string ip = "127.0.0.1";
string port = "90105";
unsigned int num_threads = 10;

int main(void) {
  Logger::log("Starting Folium Server v" + version);

  // Dispatcher::create_threads(num_threads);

  // Gateway::listen(ip, port);

  Logger::log("Done! Closing Folium Server.");
}