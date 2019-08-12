#ifndef __COMMON__
#define __COMMON__
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <map>
#include <utility>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <signal.h>
#include <math.h>

#ifdef DEBUG
#define COUT std::cout 
#else
#define COUT 0 && std::cout
#endif

using namespace std;
void split(const string& s,vector<string>& sv,const char flag = ' ');

enum COMMAND{
    GETBACKUP  = 101,
    GETVALID,
    CD,
    PUSH,
    POLL,
    PREPARE,
    FILEINFO,
    END
};
string GetCUrTIme();
bool rm_dir(const string );
bool rm(const string);
class ShmStrList
{
private:
    //share memory
    key_t shm_ipck;
    const int perm;
    const int shm_size;
    int shm_id;
    void *shm_ptr;
    int shm_name;

    //named sem
    sem_t *sem;
    string sem_name;

    //

public:
    ShmStrList(const int &shm_name = 66, const string &sem_name = "test", const int &perm = 0666, const int &shm_size = 8192):perm(perm),shm_size(shm_size)
    {
        this->shm_name = shm_name;
        this->sem_name = sem_name;
        sem = NULL;
    }
    void InitShm(const int &, const string &);
    void GetPtr();
    bool Exist(const string &);
    bool GetPid(const string &, string &);
    bool Insert(const string &);
    bool ShmRemove(const string &);
    void ShowAll();
    void ShmDt();
    void ShmRm();
    ~ShmStrList(){shmdt(shm_ptr);}
};



bool ReadLine(const int &fd, string &str);
bool ReadData(int conn, string &str, int &len);
bool WriteData(int conn, string &str);



#endif