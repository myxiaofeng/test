#include "checksum.h"
#include "md5.h"
#include "crc32.h"

CheckSum *Md5Factory::CreateCheckSum(){
    cs = new MD5();
    return cs;
}

CheckSum *Crc32Factory::CreateCheckSum(){
    cs = new Crc32();
    return cs;
}