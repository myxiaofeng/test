#ifndef __BIGFILEDEAL__
#define __BIGFILEDEAL__

#include "common.h"


struct FilePos{
    unsigned int start;
    unsigned int end;
};
struct BigFileSec{
    int fd;//file fd    
    int start;
    int end;
    int conn;
    bool push;
    int num;
};



class BigFileDeal{
private:
    
    int listenfd;
    int *conn;

    int fd;
    int num;   
    bool push;
    int *start_pos;
    int *end_pos;
public:
    BigFileDeal(){
        num = 0;
        start_pos = NULL;
        end_pos = NULL;
        conn = NULL;
    }
    void Init(bool push, int fd, double file_size, unsigned int sec_size);
    void Finish();
    void SetNum(const int &num){conn = new int[num];}
    void Listen();
    bool Accept(int i);
    void DealBigFile();
    void CloseListen()
    {
        close(listenfd);
    }
    ~BigFileDeal(){
        if(conn)
        {
            delete[] conn;
            conn = NULL;
        }
        if(start_pos)
        {
            delete[] start_pos;
            start_pos = NULL;
        }
            
        if(end_pos)
        {
            delete[] end_pos;
            end_pos = NULL;
        }
            
    }
};
static void *pthread_deal(void *arg);





#endif