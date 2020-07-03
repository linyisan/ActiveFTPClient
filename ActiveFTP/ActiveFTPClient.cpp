#include "ActiveFTPClient.h"
#include <stdio.h>
#include <conio.h>

ActiveFTPClient::ActiveFTPClient()
{
	this->mySocket = new MySocket();
	memset(this->username, 0, sizeof(this->username));
	memset(this->password, 0, sizeof(this->password));
}

ActiveFTPClient::~ActiveFTPClient()
{
	if (mySocket)
	{
		delete mySocket;
		mySocket = NULL;
	}
}

/*
	@breif 发送命令及接收响应信息
	@cmd 发送的FTP命令
	@comment 发送的命令字节大小用strlen(cmd)而不是sizeof(cmd)
*/
char * ActiveFTPClient::SendCommandAndRecvMessage(const char * cmd)
{
	char Buf[BUF_SIZE] = { 0 };
	printf("-->%s", cmd);
	if (!(mySocket->SendPack(cmd, strlen(cmd))>0)) return NULL;
	mySocket->RecvPack(Buf);
	printf("%s", Buf);
	return Buf;
}

/*
	@brief 创建数据连接（主动模式）
	@comment 先通过BindBindAndListen()系统自动分配端口号并监听，然后构建并发送"PORT"(如"PORT 192,168,101,100,14,178")指令告知FTP服务器，
	最后调用Accept()等待FTP服务器连接到本地端口(14*256+178)
*/
bool ActiveFTPClient::CreateFTPDataConnect()
{
	char strCommand[BUF_SIZE] = { 0 };
	mySocket->CloseSocket();
	SOCKADDR_IN *addr_data = mySocket->BindAndListen(mySocket->GetLocalHostIP());	
	if (!addr_data) return false;
	int tempPort = ntohs(addr_data->sin_port);	// 获取数据连接套接字的端口
	char strAddr[1024] = { 0 }; // 192,168,101,100,14,178
	strcpy(strAddr, mySocket->GetLocalHostIP());
	ReplaceStr(strAddr, ".", ",");
	sprintf(strAddr, "%s,%d,%d",strAddr, tempPort / 256, tempPort % 256);

	sprintf(strCommand, "PORT %s\r\n", strAddr);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(200)) return false;
	return true;
}

/*
	@brief 字符串替换
	@sSrc 待替换的原字符串
	@sMatchStr 要匹配的旧字符串
	@sReplaceStr 替换成的新字符串
	@comment ReplaceStr("1377678914", "7", "2");就是把字符串"1377678914"中的"7"
	替换成"2"
*/
int ActiveFTPClient::ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr)
{
	int StringLen;
	char caNewString[64];
	char* FindPos;
	FindPos = (char *)strstr(sSrc, sMatchStr);	// (char *)指向截取后的字符串首地址
	if ((!FindPos) || (!sMatchStr))
		return -1;

	while (FindPos)
	{
		memset(caNewString, 0, sizeof(caNewString));
		StringLen = FindPos - sSrc;
		strncpy(caNewString, sSrc, StringLen);
		strcat(caNewString, sReplaceStr);
		strcat(caNewString, FindPos + strlen(sMatchStr));
		strcpy(sSrc, caNewString);	// 重组原字符串

		FindPos = (char *)strstr(sSrc, sMatchStr);
	}
	free(FindPos);
	return 0;
}

/*
	@brief 创建控制连接
	@addr_To FTP服务器IP
	@port_To FTP服务器端口
*/
bool ActiveFTPClient::CreateFTPCtrlConnect(const char * addr_To, int port_To)
{
	char strResMsg[BUF_SIZE] = { 0 };
	if (!mySocket->Connect(addr_To, port_To)) return false;
	mySocket->RecvPack(strResMsg);
	printf("%s\n", strResMsg);
	if (!mySocket->CheckResponseCode(220)) return false;
	return true;
}

/*
	@bref 输入用户名和命名
*/
void ActiveFTPClient::EnterUsernameAndPassword()
{
	printf("\n>>请输入用户名:");
	memset(this->username, 0, sizeof(this->username));
	scanf("%s", this->username);

	printf("\n>>请输入密码:");
	memset(this->password, 0, sizeof(this->password));
	char ch;
	rewind(stdin);	//接收输入用户名时的空白符'\n'
	while (true)
	{
		ch = getch();
		if (ch == '\r') break;
		this->password[strlen(this->password)] = ch;
	}
}

/*
	@brief 显示路径下的子文件夹及其文件详细信息
	@targetPath 要显示的路径，默认""为当前工作路径
*/
bool ActiveFTPClient::ShowFTPFileDirectory(char *targetPath)
{
	char strCommand[BUF_SIZE] = { 0 };
	if (!CreateFTPDataConnect()) return false;

	sprintf(strCommand, "%s %s\r\n", "LIST", targetPath);
	SendCommandAndRecvMessage(strCommand);
	if (!((mySocket->CheckResponseCode(125))
			|| (mySocket->CheckResponseCode(150)))) return false;
	if (!mySocket->Accept()) return false;

	//printf("\n>>正在获取目录:....\n");
	char dirinfo[BUF_SIZE] = { 0 };
	char ListBuf[BUF_SIZE] = { 0 };
	int nRecv = 0;
	do
	{
		memset(ListBuf, 0, sizeof(ListBuf));
		nRecv = 0;
		nRecv = mySocket->RecvPackFromClient(ListBuf);
		if (nRecv > 0)
			strcat(dirinfo, ListBuf);		// bug:若是信息太大，dirinfo会出现越界情况
	} while (nRecv >0);
	mySocket->CloseSocket();

	char strResMsg[BUF_SIZE] = { 0 };
	mySocket->RecvPack(strResMsg);
	printf("%s",strResMsg);
	if (!mySocket->CheckResponseCode(226)) return false;
	//printf("成功获取目录\n");
	printf("%s", dirinfo);
	return true;
}

/*
	@bref 文件下载
	@remoteFileName 要下载的文件
	@saveFileName 文件保存到本地的文件名(最好含绝对路径)
*/
bool ActiveFTPClient::DownloadFile(const char * remoteFileName, const char *saveFileName)
{
	char strCommand[BUF_SIZE] = { 0 };
	if (!CreateFTPDataConnect())  return false;

	//printf("\n>>准备下载文件%s\n", remoteFileName);
	sprintf(strCommand, "RETR %s\r\n", remoteFileName);
	SendCommandAndRecvMessage(strCommand);
	if (!(mySocket->CheckResponseCode(150)
		|| mySocket->CheckResponseCode(125))
		) return false;
	if (!mySocket->Accept()) return false;

	FILE *fp = fopen(saveFileName, "wb");
	if (NULL == fp) {	mySocket->CloseSocket(); return false;}
	char recvBuf[BUF_SIZE] = { 0 };
	int sz_recv = 0;
	int sz_total = 0;
	do
	{
		sz_recv = 0;
		memset(recvBuf, 0, sizeof(recvBuf));
		sz_recv = mySocket->RecvPackFromClient(recvBuf);
		if (sz_recv > 0)
		{
			fwrite(recvBuf, sz_recv, 1, fp);
			sz_total = sz_total + sz_recv;
			printf("\r>>目前已下载 %d KB", sz_total / 1000);
		}
	} while (sz_recv > 0);
	fclose(fp);
	mySocket->CloseSocket();

	putchar('\n');
	char strResMsg[BUF_SIZE] = { 0 };
	mySocket->RecvPack(strResMsg);
	printf("%s", strResMsg);
	if (!mySocket->CheckResponseCode(226)) return false;
	//printf(">>成功下载文件%s\n", remoteFileName);
	return true;
}

/*
	@brief 上传文件
	@fileName 要上传本地文件的路径
*/
bool ActiveFTPClient::UpdateFile(char * filePathName)
{
	if (!CreateFTPDataConnect())  return false;
	char strCommand[BUF_SIZE] = { 0 };
	// 分割路径与文件名
	char fileName[30] = { 0 };
	char *p;
	strcpy(fileName, (p = strrchr(filePathName, '\\')) ? p + 1 : filePathName);

	//printf("\n>>准备上传文件%s\n", fileName);
	sprintf(strCommand, "STOR %s\r\n", fileName);
	SendCommandAndRecvMessage(strCommand);
	if (!(mySocket->CheckResponseCode(150)
		|| mySocket->CheckResponseCode(125))
		) return false;

	if(!mySocket->Accept()) return false;

	FILE *fp = fopen(filePathName, "rb");
	if (NULL == fp) { mySocket->CloseSocket(); return false; }
	fseek(fp, 0, SEEK_END);
	long sz_file = ftell(fp);	// 文件大小
	char sendBuf[BUF_SIZE] = { 0 };
	int sz_sent = 0;	// 到目前为止总共发送的字节大小
	int nSend = 0;	// 本次发送的字节大小
	int sz_read = 0;	// 本次实际读入字节大小
	rewind(fp);
	do
	{
		memset(sendBuf, 0, sizeof(sendBuf));
		if (0 != feof(fp))	break; // 文件指针到末尾
		sz_read = fread(sendBuf, sizeof(char), sizeof(sendBuf), fp); // 返回值是实际读入数据块的个数，每块一个char字节
		nSend = mySocket->SendPackToClient(sendBuf, sz_read);
		sz_sent = sz_sent + nSend;
		printf("\r>>目前上传进度:%3.1f%%", 1.0 * sz_sent / sz_file *100);
	} while (nSend > 0);
	fclose(fp);
	mySocket->CloseSocket();

	putchar('\n');
	char strResMsg[BUF_SIZE] = { 0 };
	mySocket->RecvPack(strResMsg);
	printf("%s", strResMsg);
	if (!mySocket->CheckResponseCode(226)) return false;
	//printf(">>成功上传文件%s\n", fileName);
	return true;
}

/*
	@brief 删除FTP服务器上的文件
	@remoteFileName 要删除的远程文件名
*/
bool ActiveFTPClient::DeleteRemoteFile(char * remoteFileName)
{
	char strCommand[BUF_SIZE] = { 0 };
	sprintf(strCommand, "DELE %s\r\n", remoteFileName);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(250)) return false;
	return true;
}

/*
	@brief 文件重命名
	@oldFileName 要进行重命名的旧文件名
	@newFileName 新文件名
*/
bool ActiveFTPClient::RenameRemoteFile(char *oldFileName, char *newFileName)
{
	char strCommand[BUF_SIZE] = { 0 };
	sprintf(strCommand, "RNFR %s\r\n", oldFileName);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(350)) return false;
	sprintf(strCommand, "RNTO %s\r\n", newFileName);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(250)) return false;
	return true;
}

/*
	@brief 创建新文件夹
	@newDirectoryName 要创建的新文件夹名
*/
bool ActiveFTPClient::CreateFTPDirectory(char * newDirectoryName)
{
	char strCommand[BUF_SIZE] = { 0 };
	sprintf(strCommand, "MKD %s\r\n", newDirectoryName);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(257)) return false;
	return true;
}

/*
	@brief （只能）删除空文件夹
	@remoeDirectoryName 要删除的空文件夹名
*/
bool ActiveFTPClient::DeleteEmptyFTPDirectory(char * remoteDirectoryName)
{
	char strCommand[BUF_SIZE] = { 0 };
	sprintf(strCommand, "RMD %s\r\n", remoteDirectoryName);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(250)) return false;
	return true;
}

/*
	@brief 切换FTP服务器（工作）路径
	@targetPath 切换到的目标路径
*/
bool ActiveFTPClient::ChangeFTPWorkingDirectory(char * targetPath)
{
	char strCommand[BUF_SIZE] = { 0 };
	sprintf(strCommand, "CWD %s\r\n", targetPath);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(250)) return false;
	return true;
}

/*
	@brief 显示目前所在（工作）路径
*/
void ActiveFTPClient::ShowFTPWorkingDirectory()
{
	SendCommandAndRecvMessage("PWD\r\n");
}

/*
	@brief 发送"QUIT"命令，释放所有套接字，结束FTP连接
*/
void ActiveFTPClient::EndFTPSession()
{
	SendCommandAndRecvMessage("QUIT\r\n");
	mySocket->CloseSocket(true);
}

/*
	@brief 设置文件传输类型
	@isASCII 若false，使用ascii类型；否则，使用binary（二进制）类型
*/
bool ActiveFTPClient::SetFileTransferType(bool isBinary)
{
	if (isBinary) 
		SendCommandAndRecvMessage("TYPE I\r\n");
	else
		SendCommandAndRecvMessage("TYPE A\r\n");
	if (!mySocket->CheckResponseCode(200)) return false;
	return false;
}

/*
	@brief 发送"USER""PASS"指令登录到FTP服务器
	@isAnonymous 是否匿名登录
	@username 用户名
	@password 密码
*/
bool ActiveFTPClient::LoginFTPServer(bool isAnonymous)
{
	char strCommand[BUF_SIZE] = { 0 };
	char tempUsername[50] = { 0 };
	char tempPassword[50] = { 0 };
	if (isAnonymous)
	{
		// 匿名登录
		//printf(">>启用匿名登录\n");
		sprintf(tempUsername, "%s", "anonymous");		
		sprintf(tempPassword, "%s", "anonymous");
	}
	else
	{
		// 用户登录
		EnterUsernameAndPassword();
		sprintf(tempUsername, "%s", this->username);
		sprintf(tempPassword, "%s", this->password);
	}

	sprintf(strCommand, "USER %s\r\n", tempUsername);
	 SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(331)) return false;

	memset(strCommand, 0, sizeof(strCommand));
	sprintf(strCommand, "PASS %s\r\n", tempPassword);
	SendCommandAndRecvMessage(strCommand);
	if (!mySocket->CheckResponseCode(230)) return false;
	return true;
}

