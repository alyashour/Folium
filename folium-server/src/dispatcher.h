/**
 * @file dispatcher.h
 * @brief Dispatcher module for managing thread pools in the Folium server.
 *
 * This file defines the interface for creating and managing thread pools, which
 * are used to handle concurrent tasks efficiently in the server. The dispatcher
 * ensures that tasks are distributed across threads for optimal performance.
 *
 * @section Responsibilities
 * - Create and manage thread pools.
 * - Handle concurrency for server operations.
 */

#ifndef FOLSERV_DISPATCHER_H_
#define FOLSERV_DISPATCHER_H_

namespace Dispatcher {
    /**
     * @brief Creates a thread pool with the specified number of threads.
     * @param num_threads The number of threads to create in the thread pool.
     * @return True if the thread pool was successfully created, false otherwise.
     * @throws std::runtime_error if the thread pool cannot be created.
     */
    bool create_threads(unsigned int num_threads);
}

#endif // FOLSERV_DISPATCHER_H_