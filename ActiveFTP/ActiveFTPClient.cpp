#include "ActiveFTPClient.h"
#include <stdio.h>
#include <conio.h>

ActiveFTPClient::ActiveFTPClient()
{
	mySocket = new MySocket();
}

/*
	@breif 发送命令及接收响应信息
	@comment 发送的命令字节大小用strlen(cmd)而不是sizeof(cmd)
*/
char * ActiveFTPClient::SendCommandAndRecvMessage(const char * cmd)
{
	char Buf[BUF_SIZE] = { 0 };
	if (!(mySocket->SendPack(cmd, strlen(cmd))>0)) return NULL;
	printf("Client:%s\n", cmd);
	mySocket->RecvPack(Buf);
	return Buf;
}

/*
	@brief 创建数据通道（主动模式）
	@comment 先通过Bind()系统自动分配端口号，然后构建并发送"PORT"(如"PORT 192,168,101,100,14,178")指令告知FTP服务器，
	最后调用Accept()等待FTP服务器连接到本地端口(14*256+178)
*/
bool ActiveFTPClient::CreateFTPDataConnect()
{
	char *strResMsg;
	char strCommand[BUF_SIZE] = { 0 };

	SOCKADDR_IN addr_data = *mySocket->Bind(mySocket->GetLocalHostIP());
	int tempPort = ntohs(addr_data.sin_port);
	char strAddr[1024] = { 0 }; // 192,168,101,100,14,178
	strcpy(strAddr, mySocket->GetLocalHostIP());
	ReplaceStr(strAddr, ".", ",");
	char strPort1[4] = { 0 };	// tempPort = strPort1 * 256 + strPort2;
	char strPort2[4] = { 0 };
	strcat(strAddr, ",");
	itoa(tempPort / 256, strPort1, 10);
	strcat(strAddr, strPort1);
	itoa(tempPort % 256, strPort2, 10);
	strcat(strAddr, ",");
	strcat(strAddr, strPort2);

	memcpy(strCommand, "PORT ", strlen("PORT "));
	memcpy(strCommand + strlen("PORT "), strAddr, strlen(strAddr));
	memcpy(strCommand + strlen("PORT ") + strlen(strAddr), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(200)) return false;

	if (!mySocket->Accept()) return false;
	printf(">>成功建立数据连接%d\n", tempPort);
	return true;

}

/*
	@brief 字符串替换
	@sSrc 待替换的原字符串
	@sMatchStr 要匹配的字符串
	@sReplaceStr 把匹配的字符串替换掉的字符串
*/
int ActiveFTPClient::ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr)
{
	int StringLen;
	char caNewString[64];
	char* FindPos;
	FindPos = (char *)strstr(sSrc, sMatchStr);
	if ((!FindPos) || (!sMatchStr))
		return -1;

	while (FindPos)
	{
		memset(caNewString, 0, sizeof(caNewString));
		StringLen = FindPos - sSrc;
		strncpy(caNewString, sSrc, StringLen);
		strcat(caNewString, sReplaceStr);
		strcat(caNewString, FindPos + strlen(sMatchStr));
		strcpy(sSrc, caNewString);

		FindPos = (char *)strstr(sSrc, sMatchStr);
	}
	free(FindPos);
	return 0;
}

/*
	@brief 创建控制通道
	@addr_To FTP服务器IP
	@port_To FTP服务器端口
	@isAnonymous 是否匿名登录
*/
bool ActiveFTPClient::CreateFTPCtrlConnect(const char * addr_To, int port_To, bool isAnonymous)
{
	char strResMsg[BUF_SIZE] = { 0 };
	if (!mySocket->Connect(addr_To, port_To)) return false;
	mySocket->RecvPack(strResMsg);
	printf("%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(220)) return false;
	if(isAnonymous)
	{
		if (!LoginFTPServer("anonymous", TEXT(""))) return false;	// 匿名登录
	}
	else
		{
			printf("请输入用户名:");
			char username[30];
			memset(username, 0, sizeof(username));
			scanf("%s", username);
			printf("请输入密码:");
			char password[30];
			memset(password, 0, sizeof(password));

			char ch;
			ch = getchar();//接收输入用户名时的\n
			while (true)
			{
				ch = getch();
				//putchar(ch);
				if (ch == '\r') break;
				//if (ch=='\n') break;

				password[strlen(password)] = ch;
			}

			if (!LoginFTPServer(username, password)) return false;	// 用户登录
		}
	return true;
}

/*
	@brief 获取目录
*/
bool ActiveFTPClient::GetFTPFileDirectory()
{
	char *strResMsg;
	char strCommand[BUF_SIZE] = { 0 };
	if (!CreateFTPDataConnect()) return false;

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

/*
	@bref 文件下载
	@remoteFileName 要下载的文件名
	@saveFileName 文件保存到本地的文件名(最好含绝对路径)
*/
bool ActiveFTPClient::DownloadFile(const char * remoteFileName, const char *saveFileName)
{
	char *strResMsg;
	if (!CreateFTPDataConnect())  return false;
	char strCommand[BUF_SIZE] = { 0 };
	memcpy(strCommand, "RETR ", strlen("RETR "));
	memcpy(strCommand + strlen("RETR "), remoteFileName, strlen(remoteFileName));
	memcpy(strCommand + strlen("RETR ") + strlen(remoteFileName), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!(mySocket->CheckResponseCode(150)
		|| mySocket->CheckResponseCode(125))
		) return false;

	FILE *fp = fopen(saveFileName, "wb");
	if (NULL == fp) return false;
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

/*
	@brief 上传文件
	@fileName 要上传本地文件的路径
*/
bool ActiveFTPClient::UpdateFile(char * fileName)
{
	char *strResMsg;
	if (!CreateFTPDataConnect())  return false;
	char strCommand[BUF_SIZE] = { 0 };
	memcpy(strCommand, "STOR ", strlen("STOR "));
	memcpy(strCommand + strlen("STOR "), fileName, strlen(fileName));
	memcpy(strCommand + strlen("STOR ") + strlen(fileName), "\r\n", 2);
	strResMsg = SendCommandAndRecvMessage(strCommand);
	printf("Server:%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(125)) return false;

	FILE *fp = fopen("F:\\bug.png", "rb");
	if (NULL == fp) return false;
	char sendBuf[BUF_SIZE] = { 0 };
	int nSend = 0;
	do
	{
		memset(sendBuf, 0, sizeof(sendBuf));
		fread(sendBuf, BUF_SIZE, 1, fp);
		if (!(strlen(sendBuf) > 0)) break;
		nSend = mySocket->SendPackToClient(sendBuf, sizeof(sendBuf));
	} while (nSend > 0);

	fclose(fp);
	//mySocket->RecvPack(strResMsg);
	//printf("%s\n", strResMsg);
	//if (!mySocket->CheckResponseCode(226)) return false;
	printf(">>成功上传文件%s", fileName);
	mySocket->CloseSocket();
	return true;
}

/*
	@brief 发送"USER""PASS"指令登录到FTP服务器
*/
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

