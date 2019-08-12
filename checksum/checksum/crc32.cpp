#include "crc32.h"


void Crc32::make_table(void)
{
    table_exist = true;
    for(int i = 0; i < 256; i++)
    {
        unsigned int c = static_cast<unsigned int>(i);
        for(int j = 0; j < 8; j++)
        {
            if(c & 1)
                c = POLYNOMIAL ^ (c >> 1);
            else
                c = c >> 1;
        }
        table[i] = c;  
    }
}
unsigned int Crc32::GetCrc32(const unsigned char *buf, int len, unsigned int crc)
{
    if(!table_exist) make_table();
    crc = ~crc;
    for(int i = 0; i < len; i++)
    {
        crc = (crc >> 8) ^ table[(crc^buf[i])&0xff];
    }
    return ~crc;

}
bool Crc32::GetFileCrc32(const char *filename, unsigned int *res)
{
    FILE *file;
    int len;
    unsigned char buf[1024];
    unsigned int crc = 0x0;

    if((file = fopen(filename, "rb")) == NULL)
    {
        cout << __FUNCTION__ << " " << __LINE__ << " " << filename << " ";
        perror("open err: ");
        return false;
    }
    else
    {
        while((len = fread(buf, 1, 1024, file)) != 0)
            crc = GetCrc32(buf, len, crc);
    }
    *res = crc;
    fclose(file);
    return true;
}



bool Crc32::GetCatCrc32Cur(unsigned int *crc, const char* path)
{
    FILE* fp = NULL;    
    if(!path)
        return false;
    vector<FileComDeal::FileInfo> vfinfo;
    FileComDeal::FileInfo finfo;
    unsigned char buf[1024];

    if(!FileComDeal::GetFileInfo(finfo, path))//Get fileinfo
        return false;
    if(finfo.type == FileComDeal::GE)//general file
        vfinfo.push_back(finfo);
    else//catalog
    {
        if(!FileComDeal::GetCurCatalogFile(vfinfo, path))
            return false;
    }
    if(vfinfo.size() == 0)//null catalog
    {
        dealNullFileCat(buf, finfo);
        *crc = GetCrc32(buf, 1024, *crc);
    }
    for(unsigned int i = 0; i < vfinfo.size(); i++)
    {
        FileComDeal::FileInfo finfo;
        finfo = vfinfo[i];
        if(finfo.type == FileComDeal::CT)
        {
            GetCatCrc32Cur(crc, finfo.filename.c_str());
        }
        else
        {
            if(finfo.size == 0)
            {
                dealNullFileCat(buf, finfo);
                *crc = GetCrc32(buf, 1024, *crc);
                continue;
            }
            fp = fopen(finfo.filename.c_str(), "rb");  
            if (fp == NULL) 
            {  
                cout << "fopen file: " << path << endl;
                perror("fopen err: ");
                return false;
            }
            int count;
            while ((count = fread(buf, 1, 1024, fp)) > 0) {  
                *crc = GetCrc32(buf, count, *crc);
                memset(buf, 0, 1024);
            }   
            fclose(fp); 
        }     
    }
    return true;
}


bool Crc32::GetCatCrc32(const char *catlog, unsigned int *res)
{
    string path;
    if (catlog == NULL || res == NULL) {  
        cout << __FUNCTION__ << " " << __LINE__ << " " << "filename and md5 null" << endl;
        return false;  
    }  
    if(catlog[0] != '/')
    {
        char cur[128];
        if(!getcwd(cur, sizeof(cur)))
        {
            perror("getcwd err");
            return false;
        }
        path = string(cur) + "/" + catlog;
    }
    else
        path = catlog;
  
    if(!GetCatCrc32Cur(res, path.c_str()))
        return false;  
    return true; 
}



