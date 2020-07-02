#pragma once
#include "MySocket.h"
#include <string>
#define BUF_SIZE 4096

class ActiveFTPClient
{
public:
	ActiveFTPClient();
	~ActiveFTPClient();
	bool CreateFTPCtrlConnect(const char *addr_To, int port_To=21);
	bool LoginFTPServer(bool isAnonymous = false);
	bool SetFileTransferType(bool isBinary=false);
	bool DownloadFile(const char * remoteFileName, const char *saveFileName);
	bool UpdateFile(char *filePathName);
	bool DeleteRemoteFile(char *remoteFileName);
	bool RenameRemoteFile(char *oldFileName, char *newFileName);
	bool ShowFTPFileDirectory();
	void ShowFTPWorkingDirectory();
	bool ChangeFTPWorkingDirectory(char *targetPath="");
	bool CreateFTPDirectory(char *newDirectoryName);
	bool DeleteFTPDirectory(char *remoteDirectoryName);
	void EndFTPSession();

protected:
	int ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr);
	char *SendCommandAndRecvMessage(const char *cmd);
	bool CreateFTPDataConnect();
	void EnterUsernameAndPassword();
private:
	MySocket *mySocket;
	char username[30], password[30];
};

