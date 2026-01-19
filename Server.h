#ifndef BD422DF4_F897_4555_89D2_D103D95674F7
#define BD422DF4_F897_4555_89D2_D103D95674F7

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>

const int BUFFER_SIZE{3000};

class Server
{
 public:
    Server();
    Server(const char* IP, const char* port);

    Server(const Server&) = delete;
    Server(Server&&)      = delete;

    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&)      = delete;

    ~Server();

    void startListen();

 private:
    int  startServer(const char* IP = nullptr, const char* port = "8080");
    void stopServer();

    void acceptConnection(int& new_socket);

    std::string buildResponse();
    void        sendResponse();

 private:
    struct addrinfo hints{};

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int m_sockfd;
    int m_newSocket;
};

#endif /* BD422DF4_F897_4555_89D2_D103D95674F7 */
