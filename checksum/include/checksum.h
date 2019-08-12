#ifndef __CHECKSUM__
#define __CHECKSUM__
#include <string>
#include <iostream>

using namespace std;

class CheckSum{

public:
    //virtual bool getByteCs(const string src, unsigned int length, string &res) = 0;
    virtual bool getStringCs(const string src, string &res) = 0;
    virtual bool getFileCs(const string path, string &res) = 0;
    virtual bool getCatlogCs(const string path, string &res) = 0; 
    virtual ~CheckSum(){};
};
/*easy factory */
/*
class CheckSumFactory{
public:
    CheckSum *CreateCheckSum(const string &check_name)
    {
        switch(check_name)
        {
            case "MD5":
                return new MD5();
            break;
            case "CRC32":
                return new Crc32();
            break;
            default:
                return NULL;
            break;
        }
    }
};
*/


/* factory */
class CheckSumFactory{
public:
    virtual CheckSum *CreateCheckSum() = 0;
    virtual ~CheckSumFactory(){
        ;
    };
};

class Md5Factory : public CheckSumFactory{
private:
    CheckSum *cs;
public:
    Md5Factory(){
        cs = NULL;
    }
    virtual CheckSum *CreateCheckSum();
    virtual ~Md5Factory()
    {
        if(cs)
        {
            delete cs;
            cs = NULL;
        } 
    }

};

class Crc32Factory : public CheckSumFactory{
private:
    CheckSum *cs;
public:
    Crc32Factory(){
        cs = NULL;
    }
    Crc32Factory(const Crc32Factory &) = delete;
    Crc32Factory &operator=(const Crc32Factory &) = delete;
    virtual CheckSum *CreateCheckSum();
    virtual ~Crc32Factory(){
        if(cs)
        {
            delete cs;
            cs = NULL;
        }
    }
};
#endif