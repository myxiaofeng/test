#ifndef __CRC32__
#define __CRC32__
#include <iostream>
#include <string>
#include "checksum.h"
#include "filecomdeal.h"
#include <string.h>

using namespace std;

class Crc32 : public CheckSum
{
private:
    enum{
        POLYNOMIAL = 0xEDB88320,
    };
    unsigned int table[256];
    bool table_exist;
    bool GetCatCrc32Cur(unsigned int *crc, const char* path);
    static void dealNullFileCat(unsigned char *buffer, const FileComDeal::FileInfo &finfo)
    {
        buffer[0] = finfo.type;
        for(unsigned int i = 1; i < finfo.filename.size() + 1 && i < 1024; i++)
            buffer[i] = finfo.filename[i-1];
        for(unsigned int i = finfo.filename.size() + 1; i < 1024; i++)
            buffer[i] = '\0';
    }
public:
    void make_table(void);
    unsigned int GetCrc32(const unsigned char *buf, int len, unsigned int crc = 0x0);
    bool GetFileCrc32(const char *filename, unsigned int *res);
    bool GetCatCrc32(const char *catlog, unsigned int *res);

/*common */
    virtual bool getStringCs(const string src, string &res){
        unsigned int crc;
        crc = GetCrc32(reinterpret_cast<const unsigned char *>(src.c_str()), src.size());
        res = to_string(crc);
        return true;
    }
    virtual bool getFileCs(const string path, string &res){
        unsigned int tmp; 
        if(!GetFileCrc32(path.c_str(), &tmp))
            return false;
        else
            res = to_string(tmp);
        return true;
    }
    virtual bool getCatlogCs(const string path, string &res){
        unsigned int tmp; 
        if(!GetCatCrc32(path.c_str(), &tmp))
            return false;
        else
            res = to_string(tmp);
        return true;
    }
};

#endif