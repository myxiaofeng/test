#include "bigfiledeal.h"
void BigFileDeal::Listen()
{
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)//firstly, creating a socket
	{
		perror("listenfd err:");
		exit(1);

	}
	struct sockaddr_in servaddr;//socket address
	memset(&servaddr, 0, sizeof(servaddr));//clear serveraddr
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);////监听所有网卡
	servaddr.sin_port = htons(9001);
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
	if(listen(listenfd, 10) < 0)
	{
		perror("listen err:");
		exit(1);
	}
}
bool BigFileDeal::Accept(int i)
{
    struct sockaddr_in client_addr;
    socklen_t client_len;//server address lenth
    client_len = sizeof(client_addr);
    struct pollfd pfd[1];
    pfd[0].fd = listenfd;
    pfd[0].events = POLLIN;//monitor read events
    EINTRSOLVE://the goto solve that capturing signals will lead to poll EINTR err
    if(poll(pfd, 1, -1) <= 0)//wait client to connect
    { 
        if(errno == EINTR)
            goto EINTRSOLVE;  
        //poll err
        perror("poll err");
        return -1;
    }
    else
    {
        //listen a client
        conn[i] = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);  
        return 0;
    }
}

void BigFileDeal::Init(bool push, int fd, double file_size, unsigned int sec_size)
{
    this->fd = fd;
    this->push = push;
    num = ceil(file_size / sec_size);
    conn = new int[num];
    start_pos  = new int[num];
    end_pos    = new int[num];
    int start = 0;
    for(int i = 0; i < num; i++)
    {
        if(i == num - 1)//the last one
        {
            start_pos[i] = start;
            end_pos[i] = file_size - 1;
        }
        else
        {
            start_pos[i] = start;
            end_pos[i] = start + sec_size - 1;
            start = end_pos[i] + 1;
        }
    } 
}

static void *pthread_deal(void *arg)
{
    BigFileSec fs;
    fs = *reinterpret_cast<BigFileSec *>(arg);
    if(fs.push)//push
    {
        lseek(fs.fd, fs.start, SEEK_CUR);
        int left_len = fs.end - fs.start;
        while(left_len>0)
        {
            string str;
            int len;
            if(!ReadData(fs.conn, str, len))
            {
                pthread_exit(NULL);
            }
            write(fs.fd, str.c_str(), str.size());
            left_len -= len;
        }
    }
    else//poll
    {
        int left_len = fs.end - fs.start;
        while(left_len > 0)
        {
            lseek(fs.fd, fs.start, SEEK_CUR);
            char buf[1024];
            int read_num = read(fs.fd, buf, sizeof(buf));
            write(fs.conn, buf, read_num);
            left_len -= read_num;
        }
    }
    close(fs.conn);
    pthread_exit(NULL);
}

void BigFileDeal::DealBigFile()
{
    BigFileSec file_sec;
    file_sec.fd = fd;
    file_sec.push = push;
    file_sec.num = num;
    //listen
    Listen();
    //accept
    pthread_t *pth_id = new pthread_t[num];
    for(int i = 0; i < num; i++)
    {
        file_sec.start = start_pos[i];
        file_sec.end = end_pos[i];
        file_sec.conn = conn[i];
        Accept(i);
        pthread_create(&pth_id[i], NULL, pthread_deal, &file_sec);
    }


    for(int i = 0; i < num; i++)
        pthread_join(pth_id[i], NULL);
    CloseListen();
    delete[] pth_id;
}


void BigFileDeal::Finish()
{
    delete[] start_pos;
    start_pos = NULL;
    delete[] end_pos;
    end_pos = NULL;
    delete[] conn;
    conn = NULL;
}