#ifndef __COMMAND__
#define __COMMAND__
#include "command.h"
#include "common.h"
#include "bigfiledeal.h"
using namespace std;

struct FileInfo{
    string filename;
    size_t size;
    string type;
};


class CommandServer{
private:
    string cmd;
    string arg;
    int client_socket;
    const static string OK;
    const static string NO;
    const static string CT;//catalog
    const static string GE;//general file
    enum{
        FILESECSIZE = 1000*1024*1024,
    };
    const static string BACKUPROOT;
    const static string VALIDROOT;
    const static string CLIENTA;
    const static string CLIENTB;
    string backup_file;
    string pushing_file;
    BigFileDeal big_file;

public: 
    ShmStrList shm_push;
    ShmStrList shm_poll;
    
    CommandServer()
    {
        cmd = -1;
        client_socket = -1;
        arg = -1;
        pushing_file = -1;
        backup_file = -1;
        shm_push.InitShm(66, "~");
        shm_poll.InitShm(77, ".");
        big_file.SetNum(0);
    }
    CommandServer(const CommandServer &command)
    {
        cmd = command.cmd;
    }
    ~CommandServer(){
        shm_push.ShmRm();
        shm_poll.ShmRm();
    };
    void GetClientSok(int client_socket){this->client_socket = client_socket;}
    void SetCmd(string cmd){this->cmd = cmd;}
    void SetArg(string arg){this->arg = arg;}
    void CloseSok(){close(client_socket);}
    bool ParseCommand();
    int Writen(const char *buf, int len);
    int Readn(char *buf, int len);
    vector<FileInfo> GetCurCatalogFile();
    string FileInfoToStr(vector<FileInfo> vfileinfo);
    FileInfo GetFileInfo(string filename);
    bool SendCmd(const string cmd)
    {
        COUT << "send cmd: " << cmd << endl;
        int len = strlen(cmd.c_str())+1;
        if(Writen(reinterpret_cast<char *>(&len), sizeof(len)) <= 0)
            exit(1);
        if(Writen(cmd.c_str(), len) <= 0)
            exit(1);
        return true;
    }
    bool ReadData(string &str)
    {
        char buf[1024];
        int len;
        if(Readn(reinterpret_cast<char *>(&len), 4) < 0)
        {
            return false;
        }
        if(Readn(buf, len) < 0)
        {
            return false;
        }
        COUT << "recv cmd: " << buf << endl;
        str = buf;
        return true;
    }
    bool SendData(string data)
    {
        COUT << "send data: " << data << endl;
        int len = data.size() + 1;
        if(Writen(reinterpret_cast<char *>(&len), sizeof(len)) <= 0)
            exit(1);
        if(Writen(data.c_str(), len) <= 0)
            exit(1);
        return true;
    }
    bool SendFile(char *data, int size)
    {
        int len = size;
        if(Writen(reinterpret_cast<char *>(&len), sizeof(len)) <= 0)
            return false;
        if(Writen(data, len) <= 0)
            return false;
        return true;
    }
    bool ReadFile(int fd)
    {
        int len;
        char buf[1024];
        while(1)
        {
            cout << "push_conflict: " << push_conflict << endl;
            if(push_conflict)//find push conflict
            {
                SendData(NO);
                push_conflict = false;
            }
            else
                SendData(OK);

            memset(buf, 0, sizeof(buf));
            memset(reinterpret_cast<char *>(&len), 0, sizeof(len));

            int ret;
            while(1)
            {
                if((ret = Readn(reinterpret_cast<char *>(&len), sizeof(len))) < 0)
                {
                    if(errno == EINTR)
                        continue;
                    else
                        break;
                }
                else
                    break;
            }
            if(ret < 0)
                return false;
                
            while(1)
            {
                if((ret = Readn(buf, len)) <= 0)
                {
                    if(errno == EINTR)
                        continue;
                    else
                        break;
                }
                else
                    break;
            }
            if(ret < 0)
                return false;
            if(!strcmp(buf, ".FILEEND"))
                break;
            COUT << "writefile len: " << len << endl;
            write(fd, buf, len);
        }

        return true;
    }
    bool WaitRecvCmdSuc()
    {
        if(!ReadData(cmd))
            exit(1);
        if(cmd != "RECVCMDSUC")
            return false;
        return true;
    }
    void ClearArg()
    {
        pushing_file.clear();
        backup_file.clear();
        cmd.clear();
        arg.clear();
    }
    bool Register()
    {
        COUT << "In register" << endl;
        string account;
        string passkey;
        if(!ReadData(account))
            exit(1);
        if(!ReadData(passkey))
            exit(1);
        int fd = open("/home/gaofeng/gaofeng/server/.COUNTMAG", O_APPEND|O_RDWR);
        if(fd < 0)
        {
            perror("open err: ");
            exit(1);
        }
        
        string str;
        while(ReadLine(fd, str))
        {
            vector<string> vstr;
            split(str, vstr);
            cout << vstr[0] << endl;
            if(account == vstr[0])
            {
                close(fd);
                return true;
            }
            str.clear();
        }
        COUT << "Write success" << endl;
        string user_input = account + " " + passkey + "\n";
        write(fd, user_input.c_str(), user_input.size());
        close(fd);
        return true;
    }
    bool LogIn()
    {
        string account;
        string passkey;
        if(!ReadData(account))
            exit(1);
        if(!ReadData(passkey))
            exit(1);

        string user_input = account + " " + passkey;

        int fd = open("/home/gaofeng/gaofeng/server/.COUNTMAG", O_RDWR);
        string str;
        while(ReadLine(fd, str))
        {
            COUT << str << endl;
            if(user_input == str)
            {
                close(fd);
                return true;
            }
            str.clear();
        }
        close(fd);
        return false;
    }
    bool AccountMag()
    {
        bool ret = false;
        string register_or_login;
        if(!ReadData(register_or_login))
            exit(1);

        while(!ret)
        {
            string str_quit;
            if(!ReadData(str_quit))
                exit(1);
            if(str_quit != OK)//quit
                exit(1);

            if(register_or_login == "register")
            {
                if(Register())
                    register_or_login = "login";
                SendData(NO);
                ret = false;
            } 
            else
            {
                if(!LogIn())
                {
                    SendData(NO);
                    ret = false;
                }
                else
                    ret = true;
            }
        }
        SendData(OK);
        return ret;
    }



    friend bool DealRep(CommandServer &);
    friend bool ReductFile(CommandServer &);
    typedef void(*cmd_handler)(CommandServer &);
    friend void DoGetValid(CommandServer &);
    friend void DoGetBackup(CommandServer &);
    friend void DoLs(CommandServer &);
    friend void DoCd(CommandServer &);
    friend void DoPush(CommandServer &);
    friend void DoPoll(CommandServer &);
    friend void DoQuit(CommandServer &);
    friend void DoRecvCmdSuc(CommandServer &);
    static const map<string, cmd_handler> ctrl_cmd;  

    static bool push_conflict;
};



#endif