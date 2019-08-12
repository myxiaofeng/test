#include <iostream>
#include "common.h"
#include "client.h"
int main(int argc, char *argv[])
{

    if(argc != 2)
    {
        std::cout << "Usage:[client.out|hostname]" << std::endl;
        exit(0);
    }
    Client clt;//server entity
    clt.ConnectServer(argv[1]);//init listen_socket
    
    clt.Dealing();

    clt.Close();
    exit(0);
}
