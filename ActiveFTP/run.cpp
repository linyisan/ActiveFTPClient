#include "ActiveFTPClient.h"
void start();
int main(int argc, char *argv[])
{
	start();
	system("pause");
	return 0;
}

// 菜单函数，请自定义函数名
void start()
{
	// 1. 前提：必须先创建FTP控制连接，登录后才能使用其他FTP命令
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	// 提示用户输入要连接到的局域网FTP服务器IP，不能连接到外网FTP服务器
	if (!(activeFTPClient->CreateFTPCtrlConnect("192.168.101.100"))) return;
	// 提示用户是否匿名登录FTP
	activeFTPClient->LoginFTPServer(true); // user
	
	//	2.1 列出可用功能，用户可通过"help 某一命令"来查看某一命令的说明，
	// 具体请参考IIS(WINDOWS自带)FTP，https://www.cnblogs.com/itech/archive/2009/09/11/1564743.html  
	// https://www.cnblogs.com/mingforyou/p/4103022.html

	// 2.2 提示用户输入命令（并输入将要调用的FTP函数参数），调用对应FTP函数
	// while() switch(){} ，以下请完成编写
	activeFTPClient->SetFileTransferType(true);	// ascii/binary
	activeFTPClient->UpdateFile("F:\\tcpip.pdf");	 // put
	activeFTPClient->DownloadFile("tcpip.pdf", "C:\\Users\\37186\\Desktop\\tcp\\tcpip.pdf");	// get
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