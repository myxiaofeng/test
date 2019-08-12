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
    bool dirct;
    int num;
};



class BigFileDeal{
private:
    int *conn;

    int fd;
    int num;   
    bool dirct;
    int *start_pos;
    int *end_pos;
public:
    BigFileDeal(){
        num = 0;
        start_pos = NULL;
        end_pos = NULL;
        conn = NULL;
    }
    void Init(bool , int , double , unsigned int );
    void Finish();
    void SetNum(const int &num){conn = new int[num];}
    bool Connect(int i);
    void DealBigFile();
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



#endif