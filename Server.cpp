#include "Server.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>

Server::Server()
{
    if (startServer() != 0)
    {
        std::cout << std::format("Failed to start server with PORT: {}", service);
    }
}

Server::Server(const char* IP, const char* port)
{
    if (startServer(IP, port) != 0)
    {
        std::cout << std::format("Failed to start server with PORT: {}", service);
    }
}

int Server::startServer(const char* IP, const char* port)
{
    int status{};
    int yes{1};

    // RAII for raw_res to deallocate linked list when out of scope
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> res(nullptr, &freeaddrinfo);
    struct addrinfo* raw_res{nullptr};  // Pointer to linked list of valid addrinfo

    hints             = addrinfo{};   // make sure the struct is empty
    hints.ai_family   = AF_UNSPEC;    // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    hints.ai_flags    = AI_PASSIVE;   // fill in my IP for me

    if ((status = getaddrinfo(IP, port, &hints, &raw_res)) != 0)
    {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        return status;
    }
    res.reset(raw_res);

    for (addrinfo* p = res.get(); p != nullptr; p = p->ai_next)
    {
        if ((m_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        // Auto unbinds old sockets to port. Should allow instant reuse when running. If
        // setsockopt not here, could have to waut for socket to unbind
        if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            return 1;
        }

        if (bind(m_sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(m_sockfd);
            perror("server: bind");
            continue;
        }

        if (getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV))
        {
            printf("could not resolve hostname");
        }
        else
        {
            printf("host=%s, serv=%s\n", host, service);
        }
        break;
    }
    return status;
}

void Server::startListen()
{
    if (listen(m_sockfd, 20) < 0)
    {
        printf("Socket listen failed");
        return;
    }

    printf("\n*** Listening on ADDRESS: %s, PORT: %s ***\n", host, service);

    while (true)
    {
        printf("====== Waiting for a new connection ======\n\n");
        acceptConnection(m_newSocket);

        char buffer[BUFFER_SIZE]{};

        size_t bytesReceived = read(m_newSocket, buffer, BUFFER_SIZE - 1);
        if (bytesReceived > 0) buffer[bytesReceived] = '\0';

        if (bytesReceived < 0)
        {
            printf("Failed to read bytes from client socket connection");
        }

        printf("------ Received Request from client ------\n");
        printf("%s \n", buffer);
        sendResponse();

        close(m_newSocket);
    }
}

void Server::acceptConnection(int& newSocket)
{
    struct sockaddr_storage their_addr;

    socklen_t addr_size = static_cast<socklen_t>(sizeof(their_addr));

    newSocket = accept(m_sockfd, (struct sockaddr*)&their_addr, &addr_size);

    if (newSocket < 0)
    {
        printf("Server failed to accept incoming connection from ADDRESS: %s, PORT: %s \n", host,
               service);
    }
}

std::string Server::buildResponse()
{
    std::string htmlFile =
        "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) "
        "</p></body></html>";
    std::ostringstream ss;
    ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
       << htmlFile;

    return ss.str();
}

void Server::sendResponse()
{
    size_t bytesSent{};

    bytesSent = write(m_newSocket, buildResponse().c_str(), buildResponse().size());

    if (bytesSent == buildResponse().size())
    {
        printf("------ Server Response sent to client ------\n\n");
    }
    else
    {
        printf("Error sending response to client");
    }
}

Server::~Server() { stopServer(); }

void Server::stopServer()
{
    close(m_sockfd);
    close(m_newSocket);
}
