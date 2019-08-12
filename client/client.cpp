#include "client.h"
#include "common.h"
Client::Client()
{
    server_socket = -1;
    state = LOGIN;

}

Client::Client(const Client &client)
{
    server_socket = client.server_socket; 
}

Client::~Client()
{
    ;
}
int Client::ConnectServer(const char *hostname)
{
    struct hostent *he = gethostbyname(hostname);
	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)//firstly, creating a socket
	{
		perror("listenfd err:");
		exit(1);

	}
	struct sockaddr_in servaddr;//socket address
	memset(&servaddr, 0, sizeof(servaddr));//clear serveraddr
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
	servaddr.sin_port = htons(SERVERPORT);
    if(connect(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect err");
        exit(1);
    }
    else
    {
        COUT << "connect success" << std::endl;
        return server_socket;
    }
}


void Client::Dealing()
{
    COUT << "start to deal!" << std::endl;
    //receive input command
    while(1)
    {
        switch(state)
        {
            case LOGIN:
                if(LogIn())
                    state = START;
                break;
            case START://wait client input cmd
                 if(cmd.empty())
                    ReceiveClientComand();
                 if(DoUser())//send cmd to server
                    state = WAITCMDSUC;
                 break;    
            case WAITCMDSUC:
                 state = SENDCMDSUC;
                 RunCommand(cmd);
                 break;
            case CLEARCMDARG:
                 ClearArg();
                 state = SENDCMDSUC;
                 break;
            case SENDCMDSUC:
                 SendRecvCmdSuc();
                 COUT << "finish a command" << endl;
                 state = START;
                 break;
        }
        
    }
}

int Client::Writen(const char *buf, int len)
{
    int ret = 0;
    int leftlen = len;
    int write_flag;
    while(1)
    {
	    write_flag = write(server_socket, buf, leftlen);
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


int Client::Readn(char *buf, int len)
{
    int buf_start = len;
    int ret;
    int leftlen = len;
    int read_flag;
    while(1)
    {
        read_flag = read(server_socket, buf, leftlen);
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
            if(leftlen == 0)
            {
                ret = buf_start;
                break;
            }
     	    buf+=read_flag;
        }
    }
    return ret; 
}



void Client::Close()
{
    close(server_socket);
}



void Client::ReceiveClientComand()
{
    char buf[64];
    getcwd(buf, sizeof(buf));
    COUT << "line: " << __LINE__ << " curruent workplace: " << buf << endl;
    cmd.clear();
    arg.clear();
    string str;
    vector<string> vstr;
    getline(cin, str);
    split(str, vstr);
    cmd = vstr[0];
    for(unsigned int i = 1; i < vstr.size(); i++)
    {
        if(i == 1)
            arg = vstr[i];
        else
            arg += (" " + vstr[i]);
    } 
    COUT << "ReceiveClientComand: " << cmd << endl;
    COUT << "ReceiveClientArg: " << arg << endl;
}


bool Client::RunCommand(string cmd)
{
    COUT << "Start to RunCommand: " << cmd << endl;
    if(ctrl_cmd.find(cmd) == ctrl_cmd.end())
    {
        if(cmd != "RECVCMDSUC")
        {
            cout << "Invalid Command" << endl;
            return false;
        }
    }
    else if(ctrl_cmd.at(cmd))
    {
        COUT << "Valid Command" << endl;   
        ctrl_cmd.at(cmd)(*this);//do according to command
    } 
    return true ;       
}


bool Client::ReadData(string &readstr)
{
    int len;
    
    char cmd[1024];
    memset(cmd, 0, sizeof(cmd));
    memset(reinterpret_cast<char *>(&len), 0, 4);
    if(Readn(reinterpret_cast<char *>(&len), 4) <= 0)//firstly, client should get the valid data length
        return false;
    if(Readn(cmd, len) <= 0)
        return false;
    
    readstr = cmd;
    COUT << "Read Data: " << readstr << endl;
    return true;
}

bool Client::WriteData(const string &buf)
{
    int len =buf.size()+1;
    if(Writen(reinterpret_cast<char *>(&len), 4) <= 0)//firstly, client should send the valid data length
        return false;
    if(Writen(buf.c_str(), len) <= 0)
        return false;
    COUT << "client write succsee: " << buf << endl;
    return true;
}
bool Client::SendFile(char *data, int size)
{
    string str_ok;
    if(!ReadData(str_ok))
        exit(1);
    if(str_ok == NO)
    {
        cout << "A client will push this file, maybe this file be update!" << endl;
    }
    int len;
    memset(reinterpret_cast<char *>(&len), 0, sizeof(len));
    len = size;
    if(Writen(reinterpret_cast<char *>(&len), sizeof(len)) <= 0)
        return false;
    if(Writen(data, len) <= 0)
        return false;
    return true;
}

bool Client::ReadFile(int fd)
{
    while(1)
    {
        int len;
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        memset(reinterpret_cast<char *>(&len), 0, 4);
        if(Readn(reinterpret_cast<char *>(&len), 4) <= 0)//firstly, client should get the valid data length
            return false;
        if(Readn(buf, len) <= 0)
            return false;
        if(!strcmp(buf, FILEEND.c_str()))
            break;
        COUT << "writefile len: " << len << endl;
        write(fd, buf, len);
    }

    return true;
}


bool Client::UserCmdDeal()
{
    vector<string> vstr;
    split(arg, vstr);

    if(cmd == "cdbackup" || cmd == "CDBACKUP")
    {
        if(vstr.size()!=0)
        {
            cout << "usage: cdbackup" << endl;
            return false;
        }
    }

    if(cmd == "cdvalid" || cmd == "CDVALID")
    {
        if(vstr.size()!=0)
        {
            cout << "usage: cdvalid" << endl;
            return false;
        }
    }

    if(cmd == "ls" || cmd == "LS")
    {
        if(vstr.size()!=1 || (vstr[0]!="server" && vstr[0]!="SERVER" && vstr[0]!="USER" && vstr[0]!="user"))
        {
            cout << "usage: ls|server/user" << endl;
            return false;
        }
    }

    if(cmd == "cd" || cmd == "CD")
    {
        if(vstr.size()!=2 || (vstr[0]!="server" && vstr[0]!="SERVER" && vstr[0]!="USER" && vstr[0]!="user"))
        {
            cout << "usage: cd|server/user|catalog" << endl;
            return false;
        }
    }

    if(cmd == "push" || cmd == "PUSH")
    {
        if(vstr.size()!=1)
        {
            cout << "usage: push|file" << endl;
            return false;
        }
        FileInfo fileinfo = GetFileInfo(arg);
        if(fileinfo.filename.empty())
        {
            cout << "No such file" << endl;        
            return false;
        }
    }

    if(cmd == "poll" || cmd == "POLL")
    {
        if(vstr.size()!=1)
        {
            cout << "usage: poll|file" << endl;
            return false;
        }
    }



    return true;
}



//get getcatalog cmd from user
bool Client::DoUser()
{
    if(!UserCmdDeal())
    {
        ClearArg();
        return false;
    }
    if(ctrl_cmd.find(cmd) == ctrl_cmd.end())
    {
        cout << "No such command" << endl;
        ClearArg();
        return false;
    }
    WriteData(cmd.c_str());
    WriteData(arg.c_str());
    return RecvCmdSuc();
}

FileInfo Client::GetFileInfo(string filename)
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


vector<FileInfo> Client::GetCurCatalogFile()
{
    char buf[128];
    getcwd(buf, sizeof(buf));
    COUT << "CurCatalog: " << buf << endl;
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

string Client::FileInfoToStr(const vector<FileInfo> &vfileinfo)
{
    string str;
    str.clear();
    for(unsigned int i = 0; i < vfileinfo.size(); i++)
    {
        FileInfo fileinfo = vfileinfo[i];
        str = str + fileinfo.filename + " " + to_string(fileinfo.size) + " " + fileinfo.type + "\n";
    }
    return str;
}

void Client::CommandErr()
{
    if(ctrl_cmd.find(cmd) == ctrl_cmd.end())
        cout << "No such cmd" << endl;
    else
    {
        if(cmd == "ls" || cmd == "LS")
            cout << "ls|user/server" << endl;
        if(cmd == "cd" || cmd == "CD")
            cout << "cd|user/server|catalog" << endl;
        if(cmd == "poll" || cmd == "POLL")
            cout << "poll|filename" << endl;
        if(cmd == "push" || cmd == "PUSH")
            cout << "push|filename" << endl;
        ClearArg();
    }   
}

void Client::ClearArg()
{
    cmd.clear();
    arg.clear();
    curfile.clear();
    curcatalog.clear();
    curfile.clear();
    old_curfile.clear();
    COUT << "LINE: " << __LINE__ << " delete file: " << old_curfile.c_str() << endl;
    if(rm(old_curfile.c_str()))
    {
        char buf[64];
        getcwd(buf, sizeof(buf));
        COUT << "unlink fail" << endl;
        COUT << "curcatalog: " << buf << endl;
    }
    old_curfile.clear();

}

void Client::SetCmdArg(string cmd, string arg)
{
    this->cmd = cmd;
    this->arg = arg;
}


bool Client::LogIn()
{
    string register_or_login;
    vector<string> vstr;
    cout << "please choose: register/login" << endl;
    getline(cin, register_or_login);
    split(register_or_login, vstr);
    if(vstr.size() != 1)
        return false;
    if(vstr[0] != "register" && vstr[0] != "login")
        return false;
    vstr.clear();
     COUT << "22222" << endl;
    WriteData(register_or_login);
 COUT << "33333" << endl;

    while(1)
    {
        string str1, str2;
        cout << "please input accoun: " << endl;
        getline(cin, str1);
        split(str1, vstr);
        if(vstr.size() != 1)
            continue;
        if(str1 == "q" || str1 == "quit")
        {
            WriteData(NO);
            exit(1);
        } 
        vstr.clear();
        cout << "please input passkey: " << endl;
        struct termios settings;
        tcgetattr(fileno(stdin), &settings);
        settings.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), TCSANOW, &settings);
        getline(cin, str2);
        settings.c_lflag |= ECHO;
        tcsetattr(fileno(stdin), TCSANOW, &settings);
        split(str2, vstr);
        if(vstr.size()!=1)
            continue;
        if(str2 == "q" || str2 == "quit")
        {
            WriteData(NO);
            exit(1);
        }
        WriteData(OK);
        WriteData(str1);
        WriteData(str2);

        string str_ok;
        if(!ReadData(str_ok))
            exit(1);
        if(str_ok == OK)
            return true;  
            
    }


    return true;
}



void DoGetBackup(Client &client)
{
    COUT << "DoGetBackup" << endl;
    client.SetCmdArg("ls", "server");
    client.state = client.SENDCMDSUC;
}

void DoCd(Client &client)
{
    
    COUT << "DoCd" << endl;
    string str;
    vector<string> vstr;
    client.split(client.arg, vstr);
    if(vstr[0] == "SERVER" || vstr[0] == "server")
    {
        if(!client.ReadData(str))
            exit(1);
    }
    else if(vstr[0] == "USER" || vstr[0] == "user")
    {
        FileInfo fileinfo;
        fileinfo = client.GetFileInfo(vstr[1]);
        if(fileinfo.filename.empty())
        {
            cout << "No such catalog: " << vstr[1] << endl;
            client.state = client.CLEARCMDARG;
            return;
        }
        chdir(fileinfo.filename.c_str());
        vector<FileInfo> vfileinfo;
        vfileinfo = client.GetCurCatalogFile();
        str = client.FileInfoToStr(vfileinfo);
    }
    cout << str;
    client.state = client.CLEARCMDARG;
}
void PushRecur(Client &client, string catalog)
{
    if(chdir(catalog.c_str())<0)//change to this catalog
    {
        COUT << "Change cataolog fail" << endl;
    }
    vector<FileInfo> vfileinfo;
    vfileinfo = client.GetCurCatalogFile();
    string str = client.FileInfoToStr(vfileinfo);
    COUT << "vfileinfo file: " << str << endl;
    COUT << "vfileinfo size: " << vfileinfo.size() << endl;

    for(unsigned int i = 0; i < vfileinfo.size(); i++)
    {
        FileInfo fileinfo = client.GetFileInfo(vfileinfo[i].filename);
        client.WriteData(fileinfo.filename+" "+to_string(fileinfo.size)+" "+fileinfo.type);
        if(fileinfo.type == client.CT)
        {
            COUT << "cataolog filename: " << fileinfo.filename << endl;
            PushRecur(client, fileinfo.filename);
        }
        else//general file
        {
            struct flock flk;
            flk.l_len = 0;
            flk.l_whence = SEEK_SET;
            flk.l_pid = getpid();
            flk.l_start = 0;
            flk.l_type = F_RDLCK;
            COUT << "general filename: " << fileinfo.filename << endl;
            int fd = open(fileinfo.filename.c_str(), O_RDONLY);
            if(fd < 0)
            {
                client.WriteData(client.NO);
                perror("open: ");
                exit(1);
            }
            else
            {
                COUT << "open filename: " << fileinfo.filename << endl;
                if(fcntl(fd, F_SETLKW, &flk) == -1)
                {
                    cout << "Line: " << __LINE__;
                    perror(" fcntl err: ");
                }
                client.WriteData(client.OK);
            }       
            if(fileinfo.size <= client.FILESECSIZE)//small file
            {
                COUT << "small file" << endl;
                char buf[1024];
                memset(buf,0, sizeof(buf));
                int readnum;
                while((readnum = read(fd, buf, sizeof(buf))) > 0)
                {
                    COUT << "readnum: " << readnum << endl;
                    client.SendFile(buf, readnum);
                    memset(buf,0, sizeof(buf));
                    usleep(5000);
                }
                string str_ok;
                if(!client.ReadData(str_ok))
                    exit(1);
                if(str_ok == client.NO)
                {
                    cout << "A client will push this file, maybe this file be update!" << endl;
                }
                client.WriteData(client.FILEEND.c_str());
            } 
            else//big file
            {
                COUT << "big file" << endl;
                client.DealBf(true, fd, fileinfo.size, client.FILESECSIZE);
            }   
            
            flk.l_type = F_UNLCK;
            if(fcntl(fd, F_SETLKW, &flk) == -1)//unlock
            {
                cout << "Line: " << __LINE__;
                perror(" fcntl err: ");
            }
                    
        }
    } 
    chdir("..");
    client.WriteData(".CATALOGEND");
}

void DoPush(Client &client)
{
    COUT << "DoPush" << endl; 
    string str_ok;
    if(!client.ReadData(str_ok))
        exit(1);
    if(str_ok != client.OK)
    {
        cout << "Another client is dealing this file" << endl;
        client.state = client.CLEARCMDARG;
        return ;
    }



    FileInfo fileinfo = client.GetFileInfo(client.arg);
    client.WriteData(fileinfo.filename+" "+to_string(fileinfo.size)+" "+fileinfo.type);
    if(fileinfo.type == client.CT)//catalog file
    {
        PushRecur(client, fileinfo.filename);
    }
    else//general file
    {
        struct flock flk;
        flk.l_len = 0;
        flk.l_whence = SEEK_SET;
        flk.l_pid = getpid();
        flk.l_start = 0;
        flk.l_type = F_RDLCK;
        int fd = open(fileinfo.filename.c_str(), O_RDONLY);
        if(fd < 0)
        {
            client.WriteData(client.NO);
            perror("open: ");
            exit(1);
        }
        else
        {
            COUT << "open filename: " << fileinfo.filename << endl;
            if(fcntl(fd, F_SETLKW, &flk)==-1)
            {
                cout << "Line: " << __LINE__;
                perror(" fcntl err: ");
            }
                    
            client.WriteData(client.OK);
        }
        if(fileinfo.size <= client.FILESECSIZE)//small file
        {
            char buf[1024];
            memset(buf,0, sizeof(buf));
            int readnum;
            while((readnum = read(fd, buf, sizeof(buf))) > 0)
            {
                COUT << "readnum: " << readnum << endl;
                client.SendFile(buf, readnum);
                memset(buf,0, sizeof(buf));
            }      
            string str_ok;
            if(!client.ReadData(str_ok))
                exit(1);
            if(str_ok == client.NO)
            {
                cout << "A client will push this file, maybe this file be update!" << endl;
            }
            client.WriteData(client.FILEEND.c_str());      
        }        
        else//big file
        {
            client.DealBf(true, fd, fileinfo.size, client.FILESECSIZE);
        } 
           
        flk.l_type = F_UNLCK;
        if(fcntl(fd, F_SETLKW, &flk)==-1)
        {
            cout << "Line: " << __LINE__;
            perror(" fcntl err: ");
        }
                    
        close(fd);  
    }

    
    client.WriteData(client.ALLEND.c_str());
    client.state = client.CLEARCMDARG;
}

void PollRecur(Client &client)
{
    string str;
    if(!client.ReadData(str))
    {
        ReductFile(client);
        exit(1);
    }
    vector<FileInfo> vfileinfo;
    vector<string> vstr;
    client.split(str, vstr);
    for(unsigned int i = 0; i < vstr.size(); i+=3)
    {
        FileInfo fileinfo;
        fileinfo.filename = vstr[i];
        fileinfo.size = stoi(vstr[i+1]);
        fileinfo.type = vstr[i+2];    
        vfileinfo.push_back(fileinfo);
    }


    COUT << "vfileinfo.size: " << vfileinfo.size() << endl;
    
    for(unsigned int i = 0; i < vfileinfo.size(); i++)
    {

        FileInfo fileinfo = vfileinfo[i];
        string str = fileinfo.filename + " " + to_string(fileinfo.size) + " " + fileinfo.type;
        client.WriteData(str);
        if(fileinfo.type == client.CT)
        {
            mkdir(fileinfo.filename.c_str(), 0777);
            COUT << "mkdirfile: " << fileinfo.filename << endl;
            chdir(fileinfo.filename.c_str());
            PollRecur(client);
            
        }
        else
        {
            COUT << "filetype: " << client.GE << endl;
            string str;
            if(!client.ReadData(str))
            {
                ReductFile(client);
                exit(1);
            }
                
            if(str == client.OK)
            {
                int fd = open(fileinfo.filename.c_str(), O_CREAT, 0777);
                if(fd < 0)
                {
                    perror("open: ");
                    ReductFile(client);//restore file
                    exit(1);
                }
                else
                {
                    COUT << "open filename: " << fileinfo.filename << endl;

                    
                    close(fd);
                    fd = open(fileinfo.filename.c_str(), O_WRONLY);
                }   
                if(fileinfo.size <= client.FILESECSIZE)//small file
                {
                    client.ReadFile(fd);
                } 
                else//big file
                {
                    client.DealBf(false, fd, fileinfo.size, client.FILESECSIZE);
                }    
                close(fd);
            }
        }
    }
    chdir("..");
    client.WriteData(".CATALOGEND");
}

void DoPoll(Client &client)
{                  
    string rd;
    if(!client.ReadData(rd))
        exit(1);
    else if (rd == client.NO)
    {
        cout << "No such file" << endl;
        client.state = client.CLEARCMDARG;
        return;
    }

    if(!client.ReadData(rd))
        exit(1);
    else if (rd == client.NO)
    {
        cout << "Another client is dealing this file" << endl;
        client.state = client.CLEARCMDARG;
        return;
    }


    string str;
    if(!client.ReadData(str))//read fileinfo
        exit(1);
    vector<string> vstr;
    FileInfo fileinfo;
    client.split(str, vstr);
    fileinfo.filename = vstr[0];
    fileinfo.size = stoi(vstr[1]);
    fileinfo.type = vstr[2];

    if(access(fileinfo.filename.c_str(), F_OK) == 0)//file_exist
    {
        
        string str_continue;
        cout << "File exist!" << endl;
        cout << "continue: yes|no?" << endl;
        getline(cin, str_continue);
        if(str_continue == "YES" || str_continue == "yes")
        {
            client.WriteData(str.c_str());
            char buf[64];
            getcwd(buf, sizeof(buf));
            client.curcatalog = string(buf)+"/";
            client.curfile = client.curcatalog+fileinfo.filename;
            client.old_curfile = fileinfo.filename + "_old";
            rename(client.curfile.c_str(), client.old_curfile.c_str());//rename file

        }
        else
        {
            client.WriteData(client.ALLEND.c_str());
            client.state = client.CLEARCMDARG;
            return;
        } 
    }
    else
        client.WriteData(str.c_str());
       
    if(fileinfo.type == client.CT)
    {
        mkdir(fileinfo.filename.c_str(), 0777);//no problem
        //chmod(fileinfo.filename.c_str(), 0666);
        COUT << "mkdirfile: " << fileinfo.filename << endl;
        chdir(fileinfo.filename.c_str());
        PollRecur(client);
        //chdir("..");
    }
    else
    {
        COUT << "filetype: " << client.GE << endl;
        string str;
        if(!client.ReadData(str))
            exit(1);
        if(str == client.OK)
        {
            int fd = open(fileinfo.filename.c_str(), O_CREAT, 0777);
            if(fd < 0 )
            {
                perror("open err: ");
                exit(1);
            }
            close(fd);
            fd = open(fileinfo.filename.c_str(), O_WRONLY);
            if(fd < 0)
            {   
                perror("open err: ");
                ReductFile(client);
                exit(1);
            }
            else
            {
                COUT << "open filename: " << fileinfo.filename << endl;
                if(fileinfo.size <= client.FILESECSIZE)//small file
                {
                    client.ReadFile(fd);
                }
                else//big file
                {
                    client.big_file.Init(false, fd, fileinfo.size, client.FILESECSIZE);
                }
                
            } 
            close(fd);
        }

    }
    client.WriteData(client.ALLEND.c_str());
    client.state = client.CLEARCMDARG;
}



//get file catalog from server
void DoLs(Client &client)
{
    COUT << "DoLs" << endl;
    string str;
    if(client.arg == "SERVER" || client.arg == "server")
    {
         if(!client.ReadData(str))
            exit(1);
    }
    else if(client.arg == "USER" || client.arg == "user")
    {
        vector<FileInfo> vfileinfo = client.GetCurCatalogFile();
        str = client.FileInfoToStr(vfileinfo);
    }
    else
    {
        client.CommandErr();
    }
    cout << str;
    client.state = client.CLEARCMDARG;
}



void DoQuit(Client &client)
{
    COUT << "DoQuit" << endl;
    client.state = client.CLEARCMDARG;
}

void DoGetValid(Client &client)
{
    client.SetCmdArg("ls", "server");
    client.state = client.SENDCMDSUC;
}

const map<string, Client::cmd_handler> Client::ctrl_cmd = {
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
void ReductFile(Client &client)
{
    COUT << "LINE: " << __LINE__ << endl;
    COUT << "curcatalog: " << client.curcatalog << endl;
    chdir(client.curcatalog.c_str());
    COUT << "LINE: " << __LINE__ << endl;
    COUT << "curfile: " << client.curfile << endl;
    rm(client.curfile.c_str());
    COUT << "LINE: " << __LINE__ << endl;
    COUT << "old_curfile: " << client.old_curfile << endl;
    rename(client.old_curfile.c_str(), client.curfile.c_str());
    client.curcatalog.clear();
    client.curfile.clear();
    client.old_curfile.clear();
}
const string Client::OK = "ok";
const string Client::NO = "no";
const string Client::CT = "catalog";
const string Client::GE = "general";
const string Client::CLIENTA = ".CLIENTA";
const string Client::CLIENTB = ".CLIENTB";
const string Client::FILEEND = ".FILEEND";
const string Client::ALLEND = ".ALLEND";




