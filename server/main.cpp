#include <iostream>
#include "server.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "common.h"
void SigHandle(int sig)
{
    COUT << "In SigHandle" << std::endl;
    while(waitpid(-1, NULL, WNOHANG) > 0);//When there are unreclaimed child processes, while will is runing
}


void deal_user1(int sig)
{
    COUT << "In deal_user1" << endl;
    CommandServer::push_conflict = true;
    //cout << "There is a client ready to push this file, maybe current file will be invalid and back up!" << endl;

}

int main(int argc, char *argv[])
{ 
    Server sev;//server entity  
    sev.Listen();//init listen_socket
    signal(SIGCHLD, SigHandle);
    signal(SIGUSR1, deal_user1);
    while(1)//server working
    {	
       
		if(sev.Accept() < 0)
		{
			perror("accept err:");
			exit(1);
		}
        pid_t pid = fork();
        if(pid == -1)
        {
            perror("fork err");
            sev.CloseClientSok();
            continue;
        }
        else if(pid == 0)//child
        {
            sev.CloseListenSok();//close listen_socket
            //start to deal client
            sev.DealClient();      
            exit(0);
        }
        else//parent
        {
            sev.CloseClientSok();
        }
        COUT << "33333" << endl;
    }
}
