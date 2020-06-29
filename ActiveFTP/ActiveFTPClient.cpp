#include "ActiveFTPClient.h"
#include <stdio.h>

ActiveFTPClient::ActiveFTPClient()
{
	mySocket = new MySocket();
}

/*
	@breif 发送命令及接收响应
	@comment 发送的命令字节大小用strlen(cmd)而不是sizeof(cmd)
*/
char * ActiveFTPClient::SendCommandAndRecvMessage(const char * cmd)
{
	char Buf[BUF_SIZE] = { 0 };
	if (!mySocket->SendPack(cmd, strlen(cmd))) return NULL;
	printf("Client:%s\n", cmd);
	mySocket->RecvPack(Buf);
	return Buf;
}

bool ActiveFTPClient::CreateFTPDataConnect(const char * addr_me, int port_me)
{
	//if (mySocket->isConnected) return true;
	char *strResMsg;
	char strCommand[BUF_SIZE] = { 0 };

	SOCKADDR_IN addr_data = *mySocket->Bind("192.168.101.100");
	int tempPort = ntohs(addr_data.sin_port);
	int a1 = tempPort / 256;
	int a2 = tempPort % 256;
	char strAddr[1024] = { 0 };
	char strPort1[100] = { 0 };
	char strPort2[100] = { 0 };
	strcat(strAddr, "192,168,101,100,");
	itoa(a1, strPort1, 10);
	strcat(strAddr, strPort1);
	itoa(a2, strPort2, 10);
	strcat(strAddr, ",");
	strcat(strAddr, strPort2);

	memcpy(strCommand, "PORT ", strlen("PORT "));//99,84,224,117, 192,168,101,100,
	memcpy(strCommand + strlen("PORT "), strAddr, strlen(strAddr));//??
	memcpy(strCommand + strlen("PORT ") + strlen(strAddr), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(200)) return false;

	if (!mySocket->Accept()) return false;
	printf(">>成功建立数据连接%d\n", tempPort);
	return true;

}

bool ActiveFTPClient::CreateFTPCtrlConnect(const char * addr_To, int port_To)
{
	char strResMsg[BUF_SIZE] = { 0 };
	if (!mySocket->Connect(addr_To, port_To)) return false;
	mySocket->RecvPack(strResMsg);
	printf("%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(220)) return false;
	//if (!LoginFTPServer("non", "12345679")) return false;	// 用户登录
	if (!LoginFTPServer("anonymous", TEXT(""))) return false;	// 匿名登录
	return true;
}

bool ActiveFTPClient::GetFTPFileDirectory()
{
	char *strResMsg;
	char strCommand[BUF_SIZE] = { 0 };
	if (!CreateFTPDataConnect("192.168.101.100")) return false;

	memcpy(strCommand, "LIST", strlen("LIST"));
	memcpy(strCommand + strlen("LIST"), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(125)) return false;

	printf(">>正在获取目录:....\n");
	char dirinfo[BUF_SIZE] = { 0 };
	char ListBuf[BUF_SIZE] = { 0 };
	int nRecv = 0;
	do
	{
		memset(ListBuf, 0, sizeof(ListBuf));
		nRecv = 0;
		//strcat(ListBuf, mySocket->RecvPackFromClient());
		nRecv = mySocket->RecvPackFromClient(ListBuf);
		if (nRecv > 0)
			strcat(dirinfo, ListBuf);
	} while (nRecv >0);

	memset(ListBuf, 0, sizeof(ListBuf));
	mySocket->RecvPack(ListBuf);
	if (!mySocket->CheckResponseCode(226))
		return false;
	printf("%s", dirinfo);

	mySocket->CloseSocket();
	return true;
}

bool ActiveFTPClient::DownloadFile(const char * remoteFileName, const char *saveFileName)
{
	char *strResMsg;
	if (!CreateFTPDataConnect("192.168.101.100"))  return false;
	char strCommand[BUF_SIZE] = { 0 };
	memcpy(strCommand, "RETR ", strlen("RETR "));
	memcpy(strCommand + strlen("RETR "), remoteFileName, strlen(remoteFileName));
	memcpy(strCommand + strlen("RETR ") + strlen(remoteFileName), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!(mySocket->CheckResponseCode(150)
		|| mySocket->CheckResponseCode(125))
		) return false;
	//strResMsg = mySocket->RecvPack();

	//strcat(saveFileName)
	FILE *fp = fopen("C:\\Users\\37186\\Desktop\\tcp\\6.png", "wb");
	if (NULL == fp) return false;
	int nRecv = 0;
	char recvBuf[BUF_SIZE] = { 0 };
	int sz_recv = 0;
	do
	{
		sz_recv = 0;
		memset(recvBuf, 0, sizeof(recvBuf));
		sz_recv = mySocket->RecvPackFromClient(recvBuf);
		if (sz_recv > 0)
			fwrite(recvBuf, sz_recv, 1, fp);
	} while (sz_recv > 0);


	fclose(fp);
	//strResMsg = mySocket->RecvPack();
	//printf("%s\n", strResMsg);
	printf(">>成功下载文件%s", remoteFileName);
	//if (!mySocket->CheckResponseCode(226)) return false;
	mySocket->CloseSocket();
	return true;
}

bool ActiveFTPClient::LoginFTPServer(const char * username, const char * password)
{
	char *strResMsg;
	char strCommand[BUF_SIZE] = { 0 };
	memcpy(strCommand, "USER ", strlen("USER "));
	memcpy(strCommand + strlen("USER "), username, strlen(username));
	memcpy(strCommand + strlen("USER ") + strlen(username), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(331)) return false;

	//strResMsg = mySocket->RecvPack();
	if (!mySocket->CheckResponseCode(331)) return false;
	memset(strCommand, 0, sizeof(strCommand));
	memcpy(strCommand, "PASS ", strlen("PASS "));
	memcpy(strCommand + strlen("PASS "), password, strlen(password));
	memcpy(strCommand + strlen("PASS ") + strlen(password), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(230)) return false;
	return true;
}

