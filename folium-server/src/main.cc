#include "auth.h"
#include "http_gateway.h"
#include "logger.h"
#include "version.h"

using namespace std;

string ip = "127.0.0.1";
int port = 50105;
unsigned int num_threads = 10;

int main(void)
{
    Logger::log("Starting Folium Server v" + Folium::VERSION);
    Logger::logS("Folium Server v", Folium::VERSION, " (build ", Folium::BUILD_ID, " ", Folium::BUILD_DATE, ")");

    Gateway::listen(ip, port);

    Logger::log("Done! Closing Folium Server.");
    return 0;
}
