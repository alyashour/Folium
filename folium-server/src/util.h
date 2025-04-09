#ifndef FSERV_UTIL_H_
#define FSERV_UTIL_H_

#include <random>
#include <chrono>
#include <thread>

namespace util {

// Constants for wait times
constexpr double MIN_WAIT = 0.1;  // 100ms
constexpr double MAX_WAIT = 1.0;  // 1 second

/**
 * Generate a random double value between min and max (inclusive)
 */
inline double randomDouble(double min, double max) {
    static std::mt19937 generator(
        static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

/**
 * Sleep for a random duration between min and max seconds
 */
inline void randomSleep(double min = MIN_WAIT, double max = MAX_WAIT) {
    double seconds = randomDouble(min, max);
    std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
}

} // namespace util

#endif // FSERV_UTIL_H_