#include <iostream>
#include <string>
#include "md5.h"
#include <string.h>
#include "account.h"
#include "checksum.h"
#include "crc32.h"
using namespace std;

int main(int argc, char *argv[])
{
    
    string res;
    CheckSumFactory *md5_fac = new Md5Factory();
    CheckSum *md5 = md5_fac->CreateCheckSum();
    CheckSumFactory *crc32_fac = new Crc32Factory();
    CheckSum *crc32 = crc32_fac->CreateCheckSum();
    md5->getStringCs("hello word", res);
    cout << res << endl;  
    res.clear();
    md5->getFileCs("README.md", res);  
    cout << res << endl;  
    res.clear();
    md5->getFileCs("/home/gaofeng/gaofeng/md5/md5/README1.md", res);  
    cout << res << endl;  
    res.clear();
    md5->getCatlogCs("/home/gaofeng/gaofeng/md5/md5/mdd1", res);
    cout << res << endl;  
    res.clear();
    md5->getCatlogCs("/home/gaofeng/gaofeng/md5/md5/mdd2", res);
    cout << res << endl;  
    res.clear();


    
    crc32->getStringCs("hello word", res);
    cout << res << endl;  
    res.clear();
    crc32->getFileCs("/home/gaofeng/gaofeng/md5/md5/README.md", res);  
    cout << res << endl;  
    res.clear();
    crc32->getFileCs("/home/gaofeng/gaofeng/md5/md5/README1.md", res);  
    cout << res << endl;  
    res.clear();
    crc32->getCatlogCs("/home/gaofeng/gaofeng/md5/md5/mdd1", res);
    cout << res << endl;  
    res.clear();
    crc32->getCatlogCs("/home/gaofeng/gaofeng/md5/md5/mdd2", res);
    cout << res << endl;  
    res.clear();
    
    /* 
	ConnectDatabase();
	QueryDatabase1();
	InsertData();
	QueryDatabase2();
	ModifyData();
	QueryDatabase2();
	DeleteData();
	QueryDatabase2();
	FreeConnect();
    */
	return 0;
	
}
