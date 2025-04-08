#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "auth.h"
#include "logger.h"
#include "version.h"

using namespace std;

std::string ip = "127.0.0.1";
std::string port = "50105";
unsigned int num_threads = 10;

int main(void) {
  Logger::log("Starting Folium Server v" + Folium::VERSION);

  std::cout << "Folium Server v" << Folium::VERSION 
            << " (build " << Folium::BUILD_ID 
            << " " << Folium::BUILD_DATE << ")" << std::endl;

  // Seed default user "admin" with password "password" if not exists
  if (!Auth::check_credentials("admin", "password")) {
      if (Auth::registerUser("admin", "password")) {
          std::cout << "Default user 'admin' seeded." << std::endl;
      } else {
          std::cout << "Failed to seed default user 'admin'." << std::endl;
      }
  } else {
      std::cout << "Default user 'admin' already exists." << std::endl;
  }

  // Test login with default credentials
  if (Auth::check_credentials("admin", "password")) {
      Auth::login("admin");
      std::cout << "Login successful for 'admin'." << std::endl;
  } else {
      std::cout << "Login failed for 'admin'." << std::endl;
  }

  // Here you would typically start your HTTP server, e.g.:
  // Gateway::listen(ip, port);

  // For demonstration, keep the server running:
  while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  Logger::log("Done! Closing Folium Server.");
  return 0;
}
