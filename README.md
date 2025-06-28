This is a C++ client-server application using Winsock for network communication over TCP. The system allows a client to:

Send a request to the server specifying:

Number of threads

Matrix dimensions (rows × columns)

The server then:

Spawns multiple threads to generate a random integer matrix

Sends the generated matrix back to the client over a socket

Uses std::vector, std::thread, and the <random> library for thread-safe, efficient execution

The client receives and displays the matrix in a structured format.
