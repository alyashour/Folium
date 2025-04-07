#include <iostream>
#include <string>

// #include "dispatcher.h"
// #include "gateway.h"
#include "logger.h"
#include "version.h"
#include <bits/this_thread_sleep.h>

using namespace std;

string ip = "127.0.0.1";
string port = "50105";
unsigned int num_threads = 10;

int main(void) {
  Logger::log("Starting Folium Server v" + Folium::VERSION);

  std::cout << "Folium Server v" << Folium::VERSION 
                  << " (build " << Folium::BUILD_ID 
                  << " " << Folium::BUILD_DATE << ")" << std::endl;

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Dispatcher::create_threads(num_threads);

  // Gateway::listen(ip, port);

  Logger::log("Done! Closing Folium Server.");
}