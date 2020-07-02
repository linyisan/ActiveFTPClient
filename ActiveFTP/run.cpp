#include "ActiveFTPClient.h"
int main(int argc, char *argv[])
{
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	if(!(activeFTPClient->CreateFTPCtrlConnect("192.168.101.100"))) return 0;
	activeFTPClient->LoginFTPServer(true);
	activeFTPClient->SetFileTransferType(false);
	activeFTPClient->ShowFTPFileDirectory();
	activeFTPClient->ChangeFTPWorkingDirectory("tt");
	activeFTPClient->DownloadFile("a.txt","C:\\Users\\37186\\Desktop\\tcp\\a.txt");
	activeFTPClient->UpdateFile("F:\\tcpip.pdf");
	activeFTPClient->ShowFTPWorkingDirectory();
	activeFTPClient->ChangeFTPWorkingDirectory();
	activeFTPClient->ShowFTPWorkingDirectory();

	activeFTPClient->CreateFTPDirectory("testa");
	activeFTPClient->CreateFTPDirectory("testb");
	activeFTPClient->DeleteFTPDirectory("a");
	activeFTPClient->DeleteRemoteFile("tcpip.pdf");
	activeFTPClient->ShowFTPFileDirectory();
	activeFTPClient->RenameRemoteFile("6.png","c.png");

	activeFTPClient->EndFTPSession();
	printf("\n");
	system("pause");
	return 0;
}