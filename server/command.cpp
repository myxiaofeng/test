#include "command.h"



bool CommandServer::ParseCommand()
{
    COUT << "Start to ParseCommand" << endl;
    if(ctrl_cmd.find(cmd) == ctrl_cmd.end())
    {
        cout << "Invalid Command" << endl;
        return false;
    }
    else
    {
        COUT << "Valid Command" << endl;
        ctrl_cmd.at(cmd)(*this);//do according to command
    } 
    return true; 
}

int CommandServer::Writen(const char *buf, int len)
{
    int ret = 0;
    int leftlen = len;
    int write_flag;
    while(1)
    {
	    write_flag = write(client_socket, buf, leftlen);
        if(write_flag < 0)
        {
            if(errno == EINTR)
            continue;
            ret = write_flag;
            break;
        }
        else if(write_flag == 0 && len != 0)
        {
            continue;
        }
        else
        {
            leftlen = leftlen - write_flag;   
            if(leftlen == 0)
            {
                ret = len;
                break;
            }
        }
  }
  return ret;
}

int CommandServer::Readn(char *buf, int len)
{
    int buf_start = len;
    int ret;
    int leftlen = len;
    int read_flag;
    while(1)
    {
        read_flag = read(client_socket, buf, leftlen);
        if(read_flag < 0)
        {
            if(errno == EINTR)
                continue;
            ret = read_flag;
            break;
        }
        else if(read_flag == 0)
        {
            ret = strlen(buf) - buf_start;
            break;
        }
        else
        {
            leftlen = leftlen - read_flag;
            buf+=read_flag;
            if(leftlen == 0)
            {
                ret = buf_start;
                break;
            }   
        }
    }
    return ret; 
}



vector<FileInfo> CommandServer::GetCurCatalogFile()
{
    char buf[64];
    getcwd(buf, sizeof(buf));
    COUT << "LINE: " << __LINE__ << "curlog: " << buf << endl;
    vector<FileInfo> vfileinfo;
    FileInfo fileinfo;
    DIR *dir;
    struct dirent *entry;
    struct stat st;
	dir = opendir(".");
    if(dir == NULL)
    {
  	    perror("opendir err:");
  	    return vfileinfo;
    }
    else
    {
 		while((entry = readdir(dir)) != nullptr)
 	 	{ 		
            if(string(entry->d_name) == ".." || string(entry->d_name) == ".")
                continue;
            COUT << "CurCatalog: " << entry->d_name << endl;
            if(lstat(entry->d_name, &st) == -1)
            {
                perror("stat err:");
                COUT << "lstat failname: " << entry->d_name << endl;
                exit(EXIT_FAILURE);
            }
            fileinfo.filename = entry->d_name;
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
            vfileinfo.push_back(fileinfo);
        }
    }
    closedir(dir);
    return vfileinfo;
}

string CommandServer::FileInfoToStr(vector<FileInfo> vfileinfo)
{
    string str;
    while(!vfileinfo.empty())
    {
        
        FileInfo fileinfo = vfileinfo.back();
        vfileinfo.pop_back();
        str = str + fileinfo.filename + " " + to_string(fileinfo.size) + " " + fileinfo.type + "\n";
    }
    return str;
}


FileInfo CommandServer::GetFileInfo(string filename)
{
    FileInfo fileinfo;
    DIR *dir;
    struct dirent *entry;
    struct stat st;
	dir = opendir("./");
    if(dir == NULL)
    {
  	    perror("opendir err:");
  	    return fileinfo;
    }
    while((entry = readdir(dir)) != nullptr)
    { 		
        if(filename == entry->d_name)//find file
        {
            
            if(lstat(entry->d_name, &st) == -1)
            {
                perror("stat err:");
                exit(EXIT_FAILURE);
            }
            fileinfo.filename = filename;
            fileinfo.size = st.st_size;
            ////判断文件类型
            switch(st.st_mode & S_IFMT)
            {       
                case S_IFDIR:  fileinfo.type = CT;  
                break;
                default: fileinfo.type = GE;
                break;
            }
        }
    }      		
    closedir(dir);
    return fileinfo;
}


void DoGetBackup(CommandServer &command)
{
    chdir("/home/gaofeng/gaofeng/server/.BackUp");
}
void DoGetValid(CommandServer &command)
{
    chdir("/home/gaofeng/gaofeng/server/.ValidCatalog");
}
void DoLs(CommandServer &command)
{
    //when server receives this command, server scans the valid catalog and sends to client
    //before sending the valid datas, server should send file command
    COUT << "DoLs" << endl;
    
    if(command.arg != "SERVER" && command.arg != "server")
        return;
    vector<FileInfo> vfileinfo;
    //get current files catalog
    vfileinfo = command.GetCurCatalogFile();
    string str = command.FileInfoToStr(vfileinfo);
    command.SendData(str.c_str());//send data
}
void DoCd(CommandServer &command)
{
    
    COUT << "DoCd" << endl;
    string cdcatalog;
    if(command.arg.empty())
        return;
    vector<string> vstr;
    split(command.arg, vstr);
    if(vstr[0] != "SERVER" && vstr[0] != "server")
    {
        return;
    }
        
    string cdct = vstr[1];
    COUT << "cd catalog: " << cdct << endl;
    char curcatalog[1024];
    getcwd(curcatalog, sizeof(curcatalog));
    COUT << "curcatalog: " << curcatalog << endl << "cd arg: " << command.arg << endl;
    if(string(curcatalog) == ".ValidCatalog" && (cdct ==".." || cdct == ".."))
    {
        cdcatalog = "No exist catalog";
    }
    else if(chdir(cdct.c_str()) < 0)
    {
        cdcatalog = "No exist catalog";  
    }
    else
    {
        cdcatalog = command.FileInfoToStr(command.GetCurCatalogFile());
    }
    command.SendData(cdcatalog);
}

//client push file to server
void DoPush(CommandServer &command)
{
    COUT << "DoPush" << endl;
    char buf[128];
    getcwd(buf, sizeof(buf));
    string filepath = string(buf) + "/" + command.arg;
    filepath += " ";
    filepath += to_string(getpid());
    COUT << "file obsolute path: " << filepath << endl;

    command.shm_push.GetPtr();//shm get address
    if(command.shm_push.Exist(filepath.c_str()))//filename exist
    {
        //first, print current file is sending
        command.SendData(command.NO);
        pid_t pid;
        string str_pid;
        command.shm_push.GetPid(filepath.c_str(), str_pid);
        command.shm_push.ShmDt();
        pid = stoi(str_pid);
        kill(pid, SIGUSR1);
        return;
    }
    command.shm_push.Insert(filepath); 
    command.shm_push.ShmDt();


    command.shm_poll.GetPtr();//shm get address
    if(command.shm_poll.Exist(filepath.c_str()))
    {
        command.SendData(command.NO);
        command.shm_poll.ShmDt();
        return;
    }
    command.shm_poll.ShmDt();
    command.SendData(command.OK);

    command.pushing_file = command.arg;//currently transfering files
    DealRep(command);//deal repeat file
    chdir(command.VALIDROOT.c_str());//only push to VALIDROOT
    while(1)
    {
        string str;
        if(!command.ReadData(str))//get fileinfo
            exit(1);
       
        /* if(str.empty())//getdata fail
        {
            ReductFile(command);
            exit(1);
        }
        */
        if(str == ".CATALOGEND")
        {
            chdir("..");
            continue;
        }
        else if(str == ".ALLEND")
        {
            command.shm_push.GetPtr();//shm get address
            command.shm_push.ShmRemove(filepath);
            command.shm_push.ShmDt();
            return;
        }
        FileInfo fileinfo;
        vector<string> vstr;
        split(str, vstr);
        fileinfo.filename = vstr[0];
        fileinfo.size = stoi(vstr[1]);
        fileinfo.type = vstr[2];
        if(fileinfo.type == command.CT)
        {
            mkdir(fileinfo.filename.c_str(), 0777);
            COUT << "mkdirfile: " << fileinfo.filename << endl;
            chdir(fileinfo.filename.c_str());
        }
        else
        {
            string cmd_ok;
            if(!command.ReadData(cmd_ok))
                exit(1);
            if(cmd_ok == command.OK)
            {
                int fd = open(fileinfo.filename.c_str(), O_CREAT, 0777);
                if(fd < 0)
                {
                    perror("open: ");
                    ReductFile(command);
                    exit(1);
                }
                close(fd);
                if(fileinfo.size <= command.FILESECSIZE)
                {
                    COUT << "Send small file" << endl;
                    struct flock flk;
                    flk.l_len = 0;
                    flk.l_start= 0;
                    flk.l_type = F_WRLCK;
                    flk.l_whence = SEEK_SET;
                    fd = open(fileinfo.filename.c_str(), O_WRONLY);
                    if(fcntl(fd, F_SETLKW, &flk))
                    {
                        cout << "Line: " << __LINE__;
                        perror(" fcntl err: ");
                    } 
                    command.ReadFile(fd);
                    flk.l_type = F_UNLCK;
                    if(fcntl(fd, F_SETLKW, &flk))
                    {
                        cout << "Line: " << __LINE__;
                        perror(" fcntl err: ");
                    }
                    close(fd);
                    
                }
                else
                {
                    command.big_file.Init(true, fd, fileinfo.size, command.FILESECSIZE);
                    command.big_file.DealBigFile();
                    command.big_file.Finish();
                }
            }

        }
    }
}


//poll catalog 
void DoPoll(CommandServer &command)
{
    COUT << "DoPoll" << endl;
    
    FileInfo fileinfo = command.GetFileInfo(command.arg);
    if(fileinfo.filename.empty())
    {
        command.SendData(command.NO);
        return;
    }
    else
        command.SendData(command.OK);


    char buf[128];
    getcwd(buf, sizeof(buf));
    string filepath = string(buf) + "/" + command.arg;
    filepath += " ";
    filepath += to_string(getpid());
    COUT << "file obsolute path: " << filepath << endl;

    //first, check pushlist
    command.shm_push.GetPtr();//shm get address
    if(command.shm_push.Exist(filepath.c_str()))//filename exist
    {
        //first, print current file is sending
        command.SendData(command.NO);
        command.shm_push.ShmDt();
        return;
    }
    command.shm_push.ShmDt();
    command.SendData(command.OK);

    //checkout polllist
    command.shm_poll.GetPtr();//shm get address
    command.shm_poll.Insert(filepath);  
    command.shm_poll.ShmDt();
    
    command.SendData(fileinfo.filename+ " " + to_string(fileinfo.size)+ " " + fileinfo.type);
    while(1)
    {
        string str;
        if(!command.ReadData(str))//get filename from client
        {
            ReductFile(command);
            exit(1);
        }
        if(str == ".CATALOGEND")
        {
            chdir("..");
            continue;
        }
        else if(str == ".ALLEND")
            break;
        FileInfo fileinfo;
        vector<string> vstr;

        split(str, vstr);
        fileinfo.filename = vstr[0];
        COUT << "fileinfo filename: " << __LINE__ << vstr[0] << endl;
        fileinfo.size = stoi(vstr[1]);
        fileinfo.type = vstr[2];
        if(fileinfo.type == command.CT)//catalog file
        {
            string str;
            char buf[64];
            getcwd(buf, sizeof(buf));
            COUT << "LINE: " << __LINE__ << "  " << buf << endl;
            if(chdir(fileinfo.filename.c_str()) < 0)
            {
                char buf[128];
                getcwd(buf, 128);         
                COUT << "cwd: " << buf << endl;
                COUT << "change catalog fail: " << fileinfo.filename << endl;
                COUT << "file type: " << fileinfo.type << endl;
                exit(1);
            }
            vector<FileInfo> vfileinfo = command.GetCurCatalogFile();
            str = command.FileInfoToStr(vfileinfo);
            COUT << "vfileinfo: " << str << endl;
            str.clear();
            for(unsigned int i = 0; i < vfileinfo.size(); i++)
            {
                str += vfileinfo[i].filename + " " + to_string(vfileinfo[i].size) + " " + vfileinfo[i].type;
                if(i != vfileinfo.size())
                {
                    str += " ";
                } 
            }
            command.SendData(str);
        }
        else//general file
        {
            struct flock flk;
            flk.l_len = 0;
            flk.l_pid = getpid();
            flk.l_start= 0;
            flk.l_type = F_RDLCK;
            flk.l_whence = SEEK_SET;
            int fd = open(fileinfo.filename.c_str(), O_RDONLY);
            if(fd < 0)
            {
                command.SendCmd(command.NO);
                char buf[128];
                getcwd(buf, 128);         
                COUT << "cwd: " << buf << endl;
                COUT << "open file fail: " << fileinfo.filename << endl;
                exit(1);
            }
            else
            {
                command.SendCmd(command.OK);
                COUT << "open file: " << fileinfo.filename << endl;
                if(fcntl(fd, F_SETLKW, &flk))//lock
                {
                    cout << "Line: " << __LINE__;
                    perror(" fcntl err: ");
                }
            }   
            if(fileinfo.size <= command.FILESECSIZE)//small file
            {   
                char buf[1024];
                memset(buf,0, sizeof(buf));
                int readnum;
                while((readnum = read(fd, buf, sizeof(buf))) > 0)
                {
                    COUT << "readnum: " << readnum << endl;
                    command.SendFile(buf, readnum);
                    memset(buf,0, sizeof(buf));
                    usleep(5000);
                }
                
            }
            
            else//big file
            {
            
                command.big_file.Init(true, fd, fileinfo.size, command.FILESECSIZE);
                command.big_file.DealBigFile();
                command.big_file.Finish();
            }
            command.SendData(".FILEEND");
            flk.l_type = F_UNLCK;
            if(fcntl(fd, F_SETLKW, &flk))//unlock
            {
                cout << "Line: " << __LINE__;
                perror(" fcntl err: ");
            }
            close(fd);
        }
    }



    COUT << "remove file: " << filepath << endl;
    command.shm_poll.GetPtr();
    command.shm_poll.ShmRemove(filepath);
    command.shm_poll.ShowAll();
    command.shm_poll.ShmDt();  
}

void DoQuit(CommandServer &command)
{  
    exit(1);
}

const map<string, CommandServer::cmd_handler> CommandServer::ctrl_cmd = {
        make_pair("cdbackup", DoGetBackup),
        make_pair("CDBACKUP", DoGetBackup),
        make_pair("cdvalid", DoGetValid),
        make_pair("CDVALID", DoGetValid),
        make_pair("ls", DoLs),
        make_pair("LS", DoLs),
        make_pair("cd", DoCd),
        make_pair("CD", DoCd),
        make_pair("push", DoPush),
        make_pair("PUSH", DoPush),
        make_pair("poll", DoPoll),
        make_pair("POLL", DoPoll),
        make_pair("quit", DoQuit),
        make_pair("QUIT", DoQuit),
};

bool DealRep(CommandServer &command)
{
    chdir(command.VALIDROOT.c_str());//change catalog to VALIDROOT
    //judge file exist?
    FileInfo fileinfo = command.GetFileInfo(command.pushing_file);
    if(fileinfo.filename.empty())//no repeat file
        return true;
    string newfile;
    string time = GetCUrTIme();//firstly, we should get time
    command.backup_file = command.BACKUPROOT + command.pushing_file + time;
    int ret = rename(command.pushing_file.c_str(), command.backup_file.c_str());
    if(ret < 0)
    {
        perror("rename err: ");
    }
    return true;
}

bool ReductFile(CommandServer &command)//reduct
{
    chdir(command.VALIDROOT.c_str());//change catalog to VALIDROOT
    int ret;
    ret = rm(command.pushing_file.c_str());//delete pushing_file
    if(ret < 0)
        return false;
    ret = rename(command.backup_file.c_str(), command.pushing_file.c_str());//reduct file
    command.pushing_file.clear();
    command.backup_file.clear();
    return true;
}





const string CommandServer::OK = "ok";
const string CommandServer::NO = "no";
const string CommandServer::CT = "catalog";
const string CommandServer::GE = "general";
const string CommandServer::VALIDROOT= "/home/gaofeng/gaofeng/server/.ValidCatalog/";
const string CommandServer::BACKUPROOT = "/home/gaofeng/gaofeng/server/.BackUp/";
bool CommandServer::push_conflict = false;
const string CommandServer::CLIENTA = ".CLIENTA";
const string CommandServer::CLIENTB = ".CLIENTB";