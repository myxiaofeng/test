#include "common.h"
#include <string>
string GetCUrTIme()
{
    time_t nowT;
    nowT = time(0);
    char strT[64];
    strftime(strT, sizeof(strT), "_%Y-%m-%d_%H:%M:%S", localtime(&nowT));
    return strT;
}

//ofstream fout("log.txt");


bool rm_dir(const string dir_full_path)
{
    DIR* dirp = opendir(dir_full_path.c_str());
    if(!dirp)
    {
        return false;
    }
    struct dirent *dir;
    struct stat st;
    while((dir = readdir(dirp)) != NULL)
    {
        if(strcmp(dir->d_name,".") == 0
                || strcmp(dir->d_name,"..") == 0)
        {
            continue;
        }
        std::string sub_path = dir_full_path + '/' + dir->d_name;
        if(lstat(sub_path.c_str(),&st) == -1)
        {
            continue;
        }
        if(S_ISDIR(st.st_mode))
        {
            if(rm_dir(sub_path) == false) // 如果是目录文件，递归删除
            {
                closedir(dirp);
                return false;
            }
            rmdir(sub_path.c_str());
        }
        else if(S_ISREG(st.st_mode))
        {
            unlink(sub_path.c_str());     // 如果是普通文件，则unlink
        }
        else
        {
            continue;
        }
    }
    if(rmdir(dir_full_path.c_str()) == -1)//delete dir itself.
    {
        closedir(dirp);
        return false;
    }
    closedir(dirp);
    return true;
}


bool rm(const string file_name)
{
    char buf[64];
    getcwd(buf, sizeof(buf));
    COUT << "cwd catalog: " << buf << " delete file: " << file_name << endl;
    std::string file_path = file_name;
    struct stat st;
    if(lstat(file_path.c_str(),&st) == -1)
    {
        return false;
    }
    if(S_ISREG(st.st_mode))
    {
        if(unlink(file_path.c_str()) == -1)
        {
            return false;
        }
    }
    else if(S_ISDIR(st.st_mode))
    {
        if(file_name == "." || file_name == "..")
        {
            return false;
        }
        if(rm_dir(file_path) == false)//delete all the files in dir.
        {
            return false;
        }
    }
    return true;
}






bool ReadData(int fd, string &readstr, int &len)
{  
    char cmd[1024];
    memset(cmd, 0, sizeof(cmd));
    memset(reinterpret_cast<char *>(&len), 0, 4);
    if(read(fd, reinterpret_cast<char *>(&len), 4) <= 0)//firstly, client should get the valid data length
        return false;
    if(read(fd, cmd, len) <= 0)
        return false;
    
    readstr = cmd;
    COUT << "Read Data: " << readstr << endl;
    return true;
}

bool WriteData(int conn, string &str, int &len){
    COUT << "send data: " << str << endl;
    len = str.size() + 1;
    if(write(conn, reinterpret_cast<char *>(&len), sizeof(len)) <= 0)
        exit(1);
    if(write(conn, str.c_str(), len) <= 0)
        exit(1);
    return true;
}
