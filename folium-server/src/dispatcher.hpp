#ifndef FOL_DISPATCHER_H
#define FOL_DISPATCHER_H

namespace Dispatcher {
    /// @brief Creates a threadpool with the specified number of threads.
    /// @throws std::runtime_error if the threadpool cannot be created.
    /// @param num_threads 
    /// @return 
    bool create_threads(unsigned int num_threads);
}

#endif // FOL_DISPATCHER_H