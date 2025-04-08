#include <string>
#include <iostream>
#include <ctime> 

#include "logger.h"

std::string static getTimestamp() {
   // get time
   time_t timestamp;
   time(&timestamp);

   // convert to string
   char timeString[std::size("Thh:mm:ssZ")];
   std::strftime(
      std::data(timeString),
      std::size(timeString),
      "%T",
      std::localtime(&timestamp)
   );

   std::string s(timeString);

   return s;
}

std::string static getHeader() {
   return "[LOG | " + getTimestamp() + "] ";
}

void Logger::log(std::string s) {
   // add a period if it doesn't end with one
   if (!s.ends_with('.')) {
      s += ".";
   }

   std::cout << getHeader() << s << std::endl;
}