#pragma once
#include "SocketClient.h"

#define BUF_SIZE 4096

class ActiveFTPClient
{
public:
	ActiveFTPClient();
	~ActiveFTPClient();

	bool CreateFTPCtrlConnect(const char *addr_To, int port_To=21);
	bool GetFTPFileDirectory();
	bool DownloadFile(const char * remoteFileName, const char *saveFileName);
	bool UpdateFile(char *fileName);

protected:
	bool GetInfoFromRemoteHost();
	bool LoginFTPServer(const char *username="", const char *password="");
	char *SendCommandAndRecvMessage(const char *cmd);
	bool CreateFTPDataConnect(const char *addr_To="192.168.101.100", int port_To= 3762);
private:
	MySocket *mySocket;
};

