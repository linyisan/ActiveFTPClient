#include "ActiveFTPClient.h"
void start();
int main(int argc, char *argv[])
{
	start();
	system("pause");
	return 0;
}

// �˵����������Զ��庯����
void start()
{
	// 1. ǰ�᣺�����ȴ���FTP�������ӣ���¼�����ʹ������FTP����
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	// ��ʾ�û�����Ҫ���ӵ��ľ�����FTP������IP���������ӵ�����FTP������
	if (!(activeFTPClient->CreateFTPCtrlConnect("192.168.101.100"))) return;
	// ��ʾ�û��Ƿ�������¼FTP
	activeFTPClient->LoginFTPServer(true); // user
	
	//	2.1 �г����ù��ܣ��û���ͨ��"help ĳһ����"���鿴ĳһ�����˵����
	// ������ο�IIS(WINDOWS�Դ�)FTP��https://www.cnblogs.com/itech/archive/2009/09/11/1564743.html  
	// https://www.cnblogs.com/mingforyou/p/4103022.html

	// 2.2 ��ʾ�û�������������뽫Ҫ���õ�FTP���������������ö�ӦFTP����
	// while() switch(){} ����������ɱ�д
	activeFTPClient->SetFileTransferType(true);	// ascii/binary
	activeFTPClient->DownloadFile("tcpip.pdf", "C:\\Users\\37186\\Desktop\\tcp\\tcpip.pdf");	// get
	activeFTPClient->UpdateFile("F:\\tcpip.pdf");	 // put
	activeFTPClient->DeleteRemoteFile("tcpip.pdf"); // delete
	activeFTPClient->RenameRemoteFile("6.png", "c.png"); // rename
	activeFTPClient->ShowFTPFileDirectory("test");	// dir
	activeFTPClient->ShowFTPWorkingDirectory(); // pwd
	activeFTPClient->ChangeFTPWorkingDirectory(); // cd
	activeFTPClient->ShowFTPWorkingDirectory();
	activeFTPClient->CreateFTPDirectory("testa"); // mkdir
	activeFTPClient->CreateFTPDirectory("testb");
	activeFTPClient->DeleteEmptyFTPDirectory("testa");		// mrdir

	activeFTPClient->ShowFTPFileDirectory();
	activeFTPClient->EndFTPSession(); // bye
}