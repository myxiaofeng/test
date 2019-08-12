#include "filecomdeal.h"

bool FileComDeal::GetFileInfo(FileComDeal::FileInfo &finfo, const string &filename)
{
    struct stat st;
    if(lstat(filename.c_str(), &st) < 0)
    {
        cout << "lstat filename: " << filename << endl;
        perror("lstat err: ");
        return false;
    }
    else
    {
        finfo.filename = filename;
        finfo.size = st.st_size;
        switch(st.st_mode & S_IFMT)
        {       
            case S_IFDIR:  finfo.type = CT;  
            break;
            default: finfo.type = GE;
            break;
        }   
    }
    return true;
}
bool FileComDeal::GetCurCatalogFile(vector<FileComDeal::FileInfo> &vfinfo, const string &filename)
{
    string curcalog = filename;
    char buf[128];
    FileInfo fileinfo;
    getcwd(buf, sizeof(buf));
    DIR *dir;
    struct dirent *entry;
    struct stat st;
	dir = opendir(filename.c_str());
    if(dir == NULL)
    {
        cout << "opendir filename: " << filename << endl;
  	    perror("opendir err:");
  	    return false;
    }
    else
    {
 		while((entry = readdir(dir)) != nullptr)
 	 	{ 		
            
            if(string(entry->d_name) == ".." || string(entry->d_name) == ".")
                continue;
            string entry_name = curcalog + "/" + entry->d_name;
            if(lstat(entry_name.c_str(), &st) == -1)
            {
                cout << "lstat filename: " << entry_name << endl;
                perror("lstat err");
                return false;
            }
            fileinfo.filename = entry_name;
            fileinfo.size = st.st_size;
            ////判断文件类型
            switch(st.st_mode & S_IFMT)
            {       
                case S_IFDIR: fileinfo.type = CT;  
                break;       
                default:  
                    fileinfo.type = GE;               
                break;   
            } 	
            vfinfo.push_back(fileinfo);
        }
    }
    closedir(dir);
    return true;   
}