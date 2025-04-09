# HTTP Response Code Cheat Sheet

## 1xx: Informational

- **100 Continue**: The server has received the request headers and the client should proceed to send the request body.
- **101 Switching Protocols**: The requester has asked the server to switch protocols and the server has agreed to do so.
- **103 Early Hints**: Used to return some response headers before final HTTP message.

## 2xx: Success

- **200 OK**: Standard responsSe for successful HTTP requests.
- **201 Created**: Request has been fulfilled and resulted in a new resource being created.
- **202 Accepted**: Request has been accepted for processing, but processing is not complete.
- **204 No Content**: Server successfully processed the request, but is not returning any content.
- **206 Partial Content**: The server is delivering only part of the resource due to a range header sent by the client.

## 3xx: Redirection

- **300 Multiple Choices**: Multiple options for the resource from which the client may choose.
- **301 Moved Permanently**: This and all future requests should be directed to the given URI.
- **302 Found**: Temporary redirect.
- **303 See Other**: The response to the request can be found under another URI using the GET method.
- **304 Not Modified**: Resource has not been modified since last requested.
- **307 Temporary Redirect**: Similar to 302, but client must keep using the same HTTP method.
- **308 Permanent Redirect**: Similar to 301, but client must keep using the same HTTP method.

## 4xx: Client Error

- **400 Bad Request**: The server cannot or will not process the request due to an apparent client error.
- **401 Unauthorized**: Authentication is required and has failed or has not been provided.
- **403 Forbidden**: The request was valid, but the server is refusing action (client doesn't have access rights).
- **404 Not Found**: The requested resource could not be found.
- **405 Method Not Allowed**: The request method is not supported for the requested resource.
- **409 Conflict**: Request could not be processed because of conflict in the request.
- **413 Payload Too Large**: The request is larger than the server is willing or able to process.
- **415 Unsupported Media Type**: The media format of the requested data is not supported by the server.
- **422 Unprocessable Entity**: The request was well-formed but was unable to be followed due to semantic errors.
- **429 Too Many Requests**: The user has sent too many requests in a given amount of time ("rate limiting").

## 5xx: Server Error

- **500 Internal Server Error**: A generic error message when an unexpected condition was encountered.
- **501 Not Implemented**: The server does not support the functionality required to fulfill the request.
- **502 Bad Gateway**: The server was acting as a gateway or proxy and received an invalid response from the upstream server.
- **503 Service Unavailable**: The server is currently unavailable (overloaded or down for maintenance).
- **504 Gateway Timeout**: The server was acting as a gateway or proxy and did not receive a timely response from the upstream server.
- **505 HTTP Version Not Supported**: The server does not support the HTTP protocol version used in the request.

## Common Usage Notes

- Use **200** for successful requests that return content
- Use **201** when a resource is created successfully
- Use **204** for successful requests that don't return content
- Use **400** for general client errors (malformed requests)
- Use **401** when authentication is required
- Use **403** when authorization fails or is forbidden
- Use **404** when a resource doesn't exist
- Use **500** for unexpected server errors