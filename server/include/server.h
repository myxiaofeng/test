#ifndef __SERVER__
#define __SERVER__
#include "command.h"
#include "common.h"

class Server{
private:
    CommandServer command;
    int listen_socket;
    int state;
    int action;
    enum{
        NORMAL = 100,
        WAITCMDSUC,
    };
    enum{
        START,
        GETCATALOG,
        SENDGETCATALOG,
        GETBACKUP,
        CD,
        PUSH,
        POLL,
        PREPARE,
        FILEINFO,
        VALIDFILE,
        QUIT,
    };
    enum {
            MAXCONN = 100,//listening 100 clients at the same time
         };
    enum {
            LISTENPORT = 9000,//listen port
         };
public:
    Server();//Default constructor
    Server(const Server &server);
    ~Server();
    void Listen();//listen function
    int Accept();//connect client
    void DealClient();//It is the most inportant function, and it is used to deal with a client
    void CloseClientSok(){command.CloseSok();}
    void CloseListenSok(){close(listen_socket);}
};

#endif
;