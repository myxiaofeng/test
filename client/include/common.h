#ifndef __COMMON__
#define __COMMON__
#include <fstream>
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
#include <vector>
#include <sstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <deque>
#include "common.h"
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
using namespace std;
/*
#ifdef DEBUG
#define COUT std::cout 

#else
#define COUT (0 && std::cout)
#endif

 */
/* 
extern ofstream fout;
#define COUT fout
*/
#define COUT 0 && std::cout
string GetCUrTIme();
bool rm_dir(const string );
bool rm(const string);

struct FileInfo{
    string filename;
    size_t size;
    string type;
};


bool ReadLine(const int &fd, string &str);
bool ReadData(int conn, string &str, int &len);
bool WriteData(int conn, string &str);







#endif