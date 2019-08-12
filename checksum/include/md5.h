#ifndef MD5_H  
#define MD5_H  
#include <iostream>
#include <string>
#include "filecomdeal.h"
#include <string.h>
#include "checksum.h"

using namespace std;

class MD5 : public CheckSum
{
private:
    typedef unsigned uint32; 
    enum {
        MD5_FILE_BUFFER_LEN = 1024, 
        SECTIONSIZE = 448
    };
    struct MD5_CTX {  
        uint32 buf[4]; //huan shu 
        uint32 bits[2];  //str changdu
        unsigned char in[64];  
    };  
    #ifndef HIGHFIRST  
    #define byteReverse(buf, len)   /* Nothing */  
    #else  
    void byteReverse(unsigned char *buf, unsigned longs);  
    
    #ifndef ASM_MD5  
    /* 
    * Note: this code is harmless on little-endian machines. 
    */  
    void byteReverse(unsigned char *buf, unsigned longs)  
    {  
        uint32 t;  
        do {  
            t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |  
            ((unsigned) buf[1] << 8 | buf[0]);  
            *(uint32 *) buf = t;  
            buf += 4;  
        }while (--longs);  
    }  
    #endif  
    #endif
    static void putu32(uint32 data, unsigned char *addr) {  
        addr[0] = (unsigned char) data;  
        addr[1] = (unsigned char) (data >> 8);  
        addr[2] = (unsigned char) (data >> 16);  
        addr[3] = (unsigned char) (data >> 24);  
    };
    static void dealNullFileCat(unsigned char *buffer, const FileComDeal::FileInfo &finfo)
    {
        buffer[0] = finfo.type;
        for(unsigned int i = 1; i < finfo.filename.size() + 1 && i < SECTIONSIZE; i++)
            buffer[i] = finfo.filename[i-1];
        for(unsigned int i = finfo.filename.size() + 1; i < SECTIONSIZE; i++)
            buffer[i] = '\0';
    }
    int getCatlogMD5Cur(MD5_CTX *ctx, const char* path); 
public:
    void MD5Init(MD5_CTX *ctx); 
    void MD5Update(MD5_CTX *ctx, unsigned char const *buf, unsigned len);  
    void MD5Final(unsigned char digest[16], MD5_CTX *ctx);  
    void MD5Transform(uint32 buf[4], uint32 const in[16]);  
    
    int getBytesMD5(const unsigned char* , unsigned int , char* );  
    int getStringMD5(const char* src, char* );  
    int getFileMD5(const char* , char* );
    int getCatlogMD5(const char* , char* );

/*common */
    virtual bool getStringCs(const string src, string &res){
        char md5[128];
        if(getStringMD5(src.c_str(), md5) < 0)
            return false;
        else
        {
            res = md5;
        }
        return true;
    }
    virtual bool getFileCs(const string path, string &res){
        char md5[128];
        if(getFileMD5(path.c_str(), md5) < 0)
            return false;
        else
            res = md5;
        return true;
    }
    virtual bool getCatlogCs(const string path, string &res){
        char md5[128];
        if(getCatlogMD5(path.c_str(), md5) < 0)
            return false;
        else
            res = md5;
        return true;
    }

};

  
#endif /* !MD5_H */ 