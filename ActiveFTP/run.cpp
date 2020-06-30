#include "ActiveFTPClient.h"
int main(int argc, char *argv[])
{
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	activeFTPClient->CreateFTPCtrlConnect("192.168.101.100",21,true);
	activeFTPClient->GetFTPFileDirectory();
	activeFTPClient->DownloadFile("6.png","C:\\Users\\37186\\Desktop\\tcp\\6.png");
	//activeFTPClient->UpdateFile("bug.png");
	printf("\n");
	system("pause");
	return 0;
}