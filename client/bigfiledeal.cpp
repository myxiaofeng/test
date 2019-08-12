#include "bigfiledeal.h"

static void *pthread_deal(void *arg)
{
    BigFileSec fs;
    fs = *reinterpret_cast<BigFileSec *>(arg);
    if(fs.dirct)//push
    {
        lseek(fs.fd, fs.start, SEEK_CUR);//set file pos
        int left_len = fs.end - fs.start + 1;
        while(left_len>0)
        {
            char buf[1024];
            int len;
            if((len = read(fs.conn, buf, sizeof(buf)) < 0))
            {
                pthread_exit(NULL);
            }
            write(fs.fd, buf, len);
            left_len -= len;
        }
    }
    else//poll
    {
        int left_len = fs.end - fs.start + 1;
        while(left_len > 0)
        {
            lseek(fs.fd, fs.start, SEEK_CUR);////set file pos
            char buf[1024];
            int read_num = read(fs.fd, buf, sizeof(buf));
            write(fs.conn, buf, read_num);
            left_len -= read_num;
        }
    }
    close(fs.conn);
    pthread_exit(NULL);
}
//Get every section file infoamation: fd, start, end and transfer direction(push
void BigFileDeal::Init(bool dirct, int fd, double file_size, unsigned int sec_size)
{
    this->fd = fd;
    this->dirct = dirct;
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

bool BigFileDeal::Connect(int i)
{
    struct hostent *he = gethostbyname("127.0.0.1");
	if((conn[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)//firstly, creating a socket
	{
		perror("listenfd err:");
		exit(1);

	}
	struct sockaddr_in servaddr;//socket address
	memset(&servaddr, 0, sizeof(servaddr));//clear serveraddr
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
	servaddr.sin_port = htons(9001);
    if(connect(conn[i], (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect err");
        exit(1);
    }
    else
    {
        COUT << "connect success" << std::endl;
        return true;
    }
}





void BigFileDeal::DealBigFile()
{
    pthread_t *pth_id = new pthread_t[num];//start num pthread
    BigFileSec file_sec;
    file_sec.fd = fd;
    file_sec.dirct = dirct;
    file_sec.num = num;
    for(int i = 0; i < num; i++)
    {
        file_sec.start = start_pos[i];
        file_sec.end = end_pos[i];
        Connect(i);
        file_sec.conn = conn[i];
        pthread_create(&pth_id[i], NULL, pthread_deal, &file_sec);
    }

    for(int i = 0; i < num; i++)
        pthread_join(pth_id[i], NULL);
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
