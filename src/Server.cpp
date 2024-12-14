#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <vector>

std::vector<std::string> processRequest(std::string *requestedString) {
    std::vector<std::string> pongResponseVector;

    //convert string to lowercase
    for (char &c: *requestedString) {
        c = static_cast<char>(std::tolower(c));
    }

    // Dereference the pointer to get the actual string
    std::stringstream requestedStream(*requestedString);
    std::string line;

    // Loop through the string, splitting by '\n'
    while (std::getline(requestedStream, line, '\n')) {
        std::cout << line << "\n";

        if(line.find("ping") != std::string::npos) {
            pongResponseVector.push_back("+PONG\r\n");
        }
    }

    return pongResponseVector;
}

void createServer(int server_fd) {
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr); // Correct type for length
    std::cout << "Waiting for a client to connect...\n";

    // Log message for debugging
    std::cout << "Logs from your program will appear here!\n";

    // Correct parentheses in the `accept` condition
    int new_socket = accept(server_fd, (sockaddr *) &client_addr, &client_addr_len);
    if (new_socket < 0) {
        perror("Accept Failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Client connected\n";

    // Buffer for reading data
    char buffer[1024] = {};

    // Read data from the client
    const ssize_t bytes_received = read(new_socket, buffer, sizeof(buffer) - 1);
    if (bytes_received < 0) {
        perror("Read Failed");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Null-terminate and print the data received
    buffer[bytes_received] = '\0';
    std::string requestedString = std::string(buffer);

    std::cout << "Received from client: " << "\n";

    std::vector<std::string> pongResponseVector = processRequest(&requestedString);


    std::cout << "Response sent to client: " << "\n";
    for(std::string response : pongResponseVector) {

        // Write and send response to the client
        const char *c_response = response.c_str();

        const ssize_t writeRequestClient = write(new_socket, c_response, response.size());
        if (writeRequestClient < 0) {
            perror("Send Response failed");
            close(new_socket);
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        std::cout << response;
    }

    // Close the client socket
    //close(new_socket);

    // Close the server socket
    //close(server_fd);
    //std::cout << "Closed connection " << "\n";
}


int main(int argc, char **argv) {
    // To test run command -> echo -ne '*1\r\n$4\r\nping\r\n' | nc localhost 6379
    // To test run command -> echo -ne 'ping\r\nping' | nc localhost 6379

    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    const int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    // Bind the socket to the network address port
    if (bind(server_fd, (sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 6379\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    while (true) {
        createServer(server_fd);
    }

    return 0;
}
