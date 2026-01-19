#include <iostream>

#include "Server.h"
int main()
{
    Server server{};
    server.startListen();
    return 0;
}
