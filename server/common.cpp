#include "common.h"

void split(const string& s,vector<string>& sv,const char flag) 
{
    sv.clear();
    istringstream iss(s);
    string temp;

    while (getline(iss, temp, flag)) 
    {
        sv.push_back(temp);
    }
    return;
}



string GetCUrTIme()
{
    time_t nowT;
    nowT = time(0);
    char strT[64];
    strftime(strT, sizeof(strT), "_%Y-%m-%d_%H:%M:%S", localtime(&nowT));
    return strT;
}


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


void ShmStrList::InitShm(const int &shm_n, const string &sem_n)
{
    this->shm_name = shm_n;
    this->sem_name = sem_n;
    //share sem
    shm_ipck = ftok("../", shm_name);
    if(shm_ipck < 0)
    {
        perror("ftok err: ");
        exit(1);
    }

    shm_id = shmget(shm_ipck, shm_size, IPC_CREAT|perm);
    if(shm_id == -1)
    {
        perror("shmget err: ");
        exit(1);
    }   
    shm_ptr = shmat(shm_id, NULL, 0);
    memset(shm_ptr, 0, shm_size);  
    //named sem
    
    sem_unlink(sem_name.c_str());
    sem = sem_open(sem_name.c_str(), O_CREAT|O_RDWR, 0666, 1);
    if(sem == SEM_FAILED)
    {
        perror("sem_open err: ");
        exit(1);
    }
    sem_close(sem);
    
}

void ShmStrList::GetPtr()
{
    shm_ptr = shmat(shm_id, NULL, 0);
    sem = sem_open(sem_name.c_str(), O_RDWR); 
    sem_wait(sem);
}

bool ShmStrList::Insert(const string &str)
{
    char *cp = (char *)shm_ptr;
    int len=0;
    int i = 0;
    while((len = strlen(cp))!=0)
    {
        cp += (len+1);
        i++;
    }
    if(!strncpy(cp, str.c_str(), str.size()+1))
    {

        cout << "strncpy err" << endl;
        return false;
        
    }
    return true;
}

bool ShmStrList::Exist(const string &str)
{
    
    char *cp = (char *)shm_ptr;
    int len = 0;
    while((len = strlen(cp)) != 0)
    {
        vector<string> vstr1, vstr2;
        split(cp, vstr1);
        split(str.c_str(), vstr2);
        if(!strcmp(vstr1[0].c_str(), vstr2[0].c_str()))
        {
            return true;
        }
        cp += (len+1);
    }
    return false;
}

bool ShmStrList::ShmRemove(const string &str)
{
    vector<string> vstr;
    char *cp = (char *)shm_ptr;
    int len = 0;
    bool delete_file = false;
    while((len = strlen(cp)) != 0)//firstly, save all file
    {
        vector<string> vstr1;
        vector<string> vstr2;
        split(cp, vstr1);
        split(str, vstr2);
        if(!delete_file)
        {
            if(!strcmp(vstr1[0].c_str(), vstr2[0].c_str()))
            {
                COUT << "No equal" << endl;
                delete_file = true;
            }
            else
            {
                COUT << "Equal" << endl;
                vstr.push_back(cp);
            }
                
        }
        else
        {
            vstr.push_back(cp);
        } 
        cp += (len+1);
    }
    memset(shm_ptr, 0, shm_size);//clear share memory
    for(unsigned int i = 0; i < vstr.size(); i++)
    {
        if(!Insert(vstr[i]))
        {
            return false;
        }
        
    }
    return true;
}



void ShmStrList::ShowAll()
{
    char *cp = (char *)shm_ptr;
    int len = 0;
    while((len = strlen(cp)) != 0)
    {
        COUT << "ShmStrList: " << cp << endl;
        cp += (len+1);
    }

}

void ShmStrList::ShmDt()
{
    sem_post(sem);
    sem_close(sem);
    shmdt(shm_ptr);
}


void ShmStrList::ShmRm()
{
    shmctl(shm_id, IPC_RMID, NULL);

}

bool ShmStrList::GetPid(const string &filepath, string &pid)
{
    char *cp = (char *)shm_ptr;
    int len = 0;
    while((len = strlen(cp)) != 0)
    {
        vector<string> vstr1, vstr2;
        split(cp, vstr1);
        split(filepath, vstr2);
        if(!strcmp(vstr1[0].c_str(), vstr2[0].c_str()))
        {
            pid = vstr1[1];
            return true;
        }
        cp += (len+1);
    }
    return false;
}




bool ReadLine(const int &fd, string &str)
{
    int ret;
    while(1)
    {
        char ch;
        ret = read(fd, &ch, 1);
        if(ret <= 0)
            return false;
        if(ch == '\n')
            break;
        str += ch;
    }
    return true;
}




bool ReadData(int conn, string &str, int &len){
    char cmd[1024];
    if(read(conn, reinterpret_cast<char *>(&len), 4) < 0)
    {
        return false;
    }
    if(read(conn, cmd, len) < 0)
    {
        return false;
    }
    COUT << "recv cmd: " << cmd << endl;
    str = cmd;
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

