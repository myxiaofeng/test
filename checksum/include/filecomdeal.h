#ifndef __FILECOMDEAL__
#define __FILECOMDEAL__
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;
class FileComDeal{
private:

public:
    struct FileInfo{
        string filename;
        size_t size;
        int type;
    };
    enum {
        CT = 0,
        GE = 1
    };
    static bool GetFileInfo(FileInfo  &finfo, const string &filename);
    static bool GetCurCatalogFile(vector<FileInfo> &vfinfo, const string &filename);
};




#endif