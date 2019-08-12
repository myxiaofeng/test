#include "server.h"
Server::Server()
{
    listen_socket = 0;
    state = NORMAL;
}

Server::Server(const Server &server)
{   
    state = server.state;
}

Server::~Server()
{
    ;
}
void Server::Listen()
{
	int listenfd;
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)//firstly, creating a socket
	{
		perror("listenfd err:");
		exit(1);

	}
	struct sockaddr_in servaddr;//socket address
	memset(&servaddr, 0, sizeof(servaddr));//clear serveraddr
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);////监听所有网卡
	servaddr.sin_port = htons(LISTENPORT);
	int on = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) < 0)
	{
		perror("setsockopt err:");
		exit(1);
	}
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind err:");
		exit(1);
	}
	if(listen(listenfd, MAXCONN) < 0)
	{
		perror("listen err:");
		exit(1);
	}
	listen_socket = listenfd;
}


int Server::Accept()
{
    COUT << "listening" << std::endl;
    int conn;
    struct sockaddr_in client_addr;
    socklen_t client_len;//server address lenth
    client_len = sizeof(client_addr);
    struct pollfd pfd[1];
    pfd[0].fd = listen_socket;
    pfd[0].events = POLLIN;//monitor read events
    EINTRSOLVE://the goto solve that capturing signals will lead to poll EINTR err
    if(poll(pfd, 1, -1) <= 0)//wait client to connect
    { 
        if(errno == EINTR)
            goto EINTRSOLVE;  
        //poll err
        perror("poll err");
        command.GetClientSok(-1);
        return -1;
    }
    else
    {
        //listen a client
        conn = accept(listen_socket, (struct sockaddr *)&client_addr, &client_len);   
        COUT << "connect with a client" << std::endl;
        command.GetClientSok(conn);
        return 0;
    }
    
}

void Server::DealClient()
{
    string cmd;
    string arg;
    COUT << "start to deal a client" << std::endl;

    
    while(!command.AccountMag());
    while(1)
    {
        if(!command.ReadData(cmd))
            exit(1);
        if(!command.ReadData(arg))
            exit(1);
        command.SendCmd("RECVCMDSUC");
        command.SetCmd(cmd);
        command.SetArg(arg);
        command.ParseCommand(); 
        command.WaitRecvCmdSuc();//everytime a command has run, needing to wait RECVCMDSUC
        command.ClearArg(); 
        cout << "finish a command" << endl;
    }
}






