#ifndef __CLIENT__
#define __CLIENT__
#include "common.h"
#include "bigfiledeal.h"
using namespace std;



class Client{
public:
    
    Client();//Default constructor
    Client(const Client &client);
    ~Client();
    int ConnectServer(const char *hostname);//listen function
    void Dealing();//It is the most inportant function, and it is used to deal with a client
    int Writen(const char *buf, int len);
    int Readn(char *buf, int len);
    void Close();
    void ReceiveClientComand();
    bool RunCommand(string cmd);
    bool DoUser();
    bool UserCmdDeal();
    bool ReadData(string &);
    bool WriteData(const string &);
    bool ReadFile(int );
    bool SendFile(char *data, int size);
    bool SendRecvCmdSuc()
    {
        return WriteData("RECVCMDSUC");
    }
    bool RecvCmdSuc()
    {
        string str;
        if(!ReadData(str))
            exit(1);
        if(str == "RECVCMDSUC")
            return true;
        else
            return false;
    }
    void split(const string& s,vector<string>& sv,const char flag = ' ') 
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
    void CommandErr();
    void SetCmdArg(string, string);
    FileInfo GetFileInfo(string filename);
    vector<FileInfo> GetCurCatalogFile();
    string FileInfoToStr(const vector<FileInfo> &vfileinfo);
    void ClearArg();
    bool LogIn();
    void DealBf(bool push, int fd, double file_size, unsigned int sec_size)
    {
        big_file.Init(push, fd, file_size, sec_size);
        big_file.DealBigFile();
        big_file.Finish();
    }
private:
    int server_socket;//peer server socket
    string arg;
    string cmd;
    enum {
            SERVERPORT = 9000,//listen port
         };
    
    typedef void(*cmd_handler)(Client &);
    friend void DoGetBackup(Client &);
    friend void DoLs(Client &);
    friend void DoCd(Client &);
    friend void PushRecur(Client &, string);
    friend void DoPush(Client &);
    friend void PollRecur(Client &);
    friend void DoPoll(Client &);
    friend void DoQuit(Client &);
    friend void DoGetValid(Client &);
    friend void ReductFile(Client &);
    static const map<string, cmd_handler> ctrl_cmd;
    int state;
    enum{
        START = 0,
        LOGIN,
        WAITCMDSUC,
        SENDCMDSUC,
        CLEARCMDARG,
    };
    const static string OK;
    const static string NO;
    const static string CT;
    const static string GE;
    const static string CLIENTA;
    const static string CLIENTB;
    const static string FILEEND;
    const static string ALLEND;
    enum{
        FILESECSIZE = 1000*1024*1024,
    };
    string curcatalog;
    string curfile;
    string old_curfile;

    BigFileDeal big_file;
};

#endif
