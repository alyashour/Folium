#ifndef FOLSERV_API_ROUTES_H_
#define FOLSERV_API_ROUTES_H_

#include "httplib.h"

/**
 * @brief Registers all API routes to the given HTTP server.
 * @param svr The HTTP server instance to attach routes to.
 */
void setup_routes(httplib::Server& svr);

#endif // FOLSERV_API_ROUTES_H_
