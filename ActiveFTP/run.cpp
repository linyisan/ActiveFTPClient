#include "ActiveFTPClient.h"
int main(int argc, char *argv[])
{
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	activeFTPClient->CreateFTPCtrlConnect("192.168.101.100");
	activeFTPClient->GetFTPFileDirectory();
	activeFTPClient->DownloadFile("6.png","");

	return 0;
}