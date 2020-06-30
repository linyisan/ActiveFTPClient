#pragma once
#include "MySocket.h"
#include <string>
#define BUF_SIZE 4096

class ActiveFTPClient
{
public:
	ActiveFTPClient();
	~ActiveFTPClient();
	bool CreateFTPCtrlConnect(const char *addr_To, int port_To=21, bool isAnonymous=true);
	bool GetFTPFileDirectory();
	bool DownloadFile(const char * remoteFileName, const char *saveFileName);
	bool UpdateFile(char *fileName);

protected:
	int ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr);
	bool GetInfoFromRemoteHost();
	bool LoginFTPServer(const char *username="", const char *password="");
	char *SendCommandAndRecvMessage(const char *cmd);
	bool CreateFTPDataConnect();
private:
	MySocket *mySocket;
};

