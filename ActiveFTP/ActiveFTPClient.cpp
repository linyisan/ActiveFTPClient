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
	@breif �������������Ӧ��Ϣ
	@cmd ���͵�FTP����
	@comment ���͵������ֽڴ�С��strlen(cmd)������sizeof(cmd)
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
	@brief �����������ӣ�����ģʽ��
	@comment ��ͨ��BindBindAndListen()ϵͳ�Զ�����˿ںŲ�������Ȼ�󹹽�������"PORT"(��"PORT 192,168,101,100,14,178")ָ���֪FTP��������
	������Accept()�ȴ�FTP���������ӵ����ض˿�(14*256+178)
*/
bool ActiveFTPClient::CreateFTPDataConnect()
{
	char strCommand[BUF_SIZE] = { 0 };
	mySocket->CloseSocket();
	SOCKADDR_IN *addr_data = mySocket->BindAndListen(mySocket->GetLocalHostIP());	
	if (!addr_data) return false;
	int tempPort = ntohs(addr_data->sin_port);	// ��ȡ���������׽��ֵĶ˿�
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
	@brief �ַ����滻
	@sSrc ���滻��ԭ�ַ���
	@sMatchStr Ҫƥ��ľ��ַ���
	@sReplaceStr �滻�ɵ����ַ���
	@comment ReplaceStr("1377678914", "7", "2");���ǰ��ַ���"1377678914"�е�"7"
	�滻��"2"
*/
int ActiveFTPClient::ReplaceStr(char* sSrc, char* sMatchStr, char* sReplaceStr)
{
	int StringLen;
	char caNewString[64];
	char* FindPos;
	FindPos = (char *)strstr(sSrc, sMatchStr);	// (char *)ָ���ȡ����ַ����׵�ַ
	if ((!FindPos) || (!sMatchStr))
		return -1;

	while (FindPos)
	{
		memset(caNewString, 0, sizeof(caNewString));
		StringLen = FindPos - sSrc;
		strncpy(caNewString, sSrc, StringLen);
		strcat(caNewString, sReplaceStr);
		strcat(caNewString, FindPos + strlen(sMatchStr));
		strcpy(sSrc, caNewString);	// ����ԭ�ַ���

		FindPos = (char *)strstr(sSrc, sMatchStr);
	}
	free(FindPos);
	return 0;
}

/*
	@brief ������������
	@addr_To FTP������IP
	@port_To FTP�������˿�
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
	@bref �����û���������
*/
void ActiveFTPClient::EnterUsernameAndPassword()
{
	printf("\n>>�������û���:");
	memset(this->username, 0, sizeof(this->username));
	scanf("%s", this->username);

	printf("\n>>����������:");
	memset(this->password, 0, sizeof(this->password));
	char ch;
	rewind(stdin);	//���������û���ʱ�Ŀհ׷�'\n'
	while (true)
	{
		ch = getch();
		if (ch == '\r') break;
		this->password[strlen(this->password)] = ch;
	}
}

/*
	@brief ��ʾ·���µ����ļ��м����ļ���ϸ��Ϣ
	@targetPath Ҫ��ʾ��·����Ĭ��""Ϊ��ǰ����·��
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

	//printf("\n>>���ڻ�ȡĿ¼:....\n");
	char dirinfo[BUF_SIZE] = { 0 };
	char ListBuf[BUF_SIZE] = { 0 };
	int nRecv = 0;
	do
	{
		memset(ListBuf, 0, sizeof(ListBuf));
		nRecv = 0;
		nRecv = mySocket->RecvPackFromClient(ListBuf);
		if (nRecv > 0)
			strcat(dirinfo, ListBuf);		// bug:������Ϣ̫��dirinfo�����Խ�����
	} while (nRecv >0);
	mySocket->CloseSocket();

	char strResMsg[BUF_SIZE] = { 0 };
	mySocket->RecvPack(strResMsg);
	printf("%s",strResMsg);
	if (!mySocket->CheckResponseCode(226)) return false;
	//printf("�ɹ���ȡĿ¼\n");
	printf("%s", dirinfo);
	return true;
}

/*
	@bref �ļ�����
	@remoteFileName Ҫ���ص��ļ�
	@saveFileName �ļ����浽���ص��ļ���(��ú�����·��)
*/
bool ActiveFTPClient::DownloadFile(const char * remoteFileName, const char *saveFileName)
{
	char strCommand[BUF_SIZE] = { 0 };
	if (!CreateFTPDataConnect())  return false;

	//printf("\n>>׼�������ļ�%s\n", remoteFileName);
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
			printf("\r>>Ŀǰ������ %d KB", sz_total / 1000);
		}
	} while (sz_recv > 0);
	fclose(fp);
	mySocket->CloseSocket();

	putchar('\n');
	char strResMsg[BUF_SIZE] = { 0 };
	mySocket->RecvPack(strResMsg);
	printf("%s", strResMsg);
	if (!mySocket->CheckResponseCode(226)) return false;
	//printf(">>�ɹ������ļ�%s\n", remoteFileName);
	return true;
}

/*
	@brief �ϴ��ļ�
	@fileName Ҫ�ϴ������ļ���·��
*/
bool ActiveFTPClient::UpdateFile(char * filePathName)
{
	if (!CreateFTPDataConnect())  return false;
	char strCommand[BUF_SIZE] = { 0 };
	// �ָ�·�����ļ���
	char fileName[30] = { 0 };
	char *p;
	strcpy(fileName, (p = strrchr(filePathName, '\\')) ? p + 1 : filePathName);

	//printf("\n>>׼���ϴ��ļ�%s\n", fileName);
	sprintf(strCommand, "STOR %s\r\n", fileName);
	SendCommandAndRecvMessage(strCommand);
	if (!(mySocket->CheckResponseCode(150)
		|| mySocket->CheckResponseCode(125))
		) return false;

	if(!mySocket->Accept()) return false;

	FILE *fp = fopen(filePathName, "rb");
	if (NULL == fp) { mySocket->CloseSocket(); return false; }
	fseek(fp, 0, SEEK_END);
	long sz_file = ftell(fp);	// �ļ���С
	char sendBuf[BUF_SIZE] = { 0 };
	int sz_sent = 0;	// ��ĿǰΪֹ�ܹ����͵��ֽڴ�С
	int nSend = 0;	// ���η��͵��ֽڴ�С
	int sz_read = 0;	// ����ʵ�ʶ����ֽڴ�С
	rewind(fp);
	do
	{
		memset(sendBuf, 0, sizeof(sendBuf));
		if (0 != feof(fp))	break; // �ļ�ָ�뵽ĩβ
		sz_read = fread(sendBuf, sizeof(char), sizeof(sendBuf), fp); // ����ֵ��ʵ�ʶ������ݿ�ĸ�����ÿ��һ��char�ֽ�
		nSend = mySocket->SendPackToClient(sendBuf, sz_read);
		sz_sent = sz_sent + nSend;
		printf("\r>>Ŀǰ�ϴ�����:%3.1f%%", 1.0 * sz_sent / sz_file *100);
	} while (nSend > 0);
	fclose(fp);
	mySocket->CloseSocket();

	putchar('\n');
	char strResMsg[BUF_SIZE] = { 0 };
	mySocket->RecvPack(strResMsg);
	printf("%s", strResMsg);
	if (!mySocket->CheckResponseCode(226)) return false;
	//printf(">>�ɹ��ϴ��ļ�%s\n", fileName);
	return true;
}

/*
	@brief ɾ��FTP�������ϵ��ļ�
	@remoteFileName Ҫɾ����Զ���ļ���
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
	@brief �ļ�������
	@oldFileName Ҫ�����������ľ��ļ���
	@newFileName ���ļ���
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
	@brief �������ļ���
	@newDirectoryName Ҫ���������ļ�����
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
	@brief ��ֻ�ܣ�ɾ�����ļ���
	@remoeDirectoryName Ҫɾ���Ŀ��ļ�����
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
	@brief �л�FTP��������������·��
	@targetPath �л�����Ŀ��·��
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
	@brief ��ʾĿǰ���ڣ�������·��
*/
void ActiveFTPClient::ShowFTPWorkingDirectory()
{
	SendCommandAndRecvMessage("PWD\r\n");
}

/*
	@brief ����"QUIT"����ͷ������׽��֣�����FTP����
*/
void ActiveFTPClient::EndFTPSession()
{
	SendCommandAndRecvMessage("QUIT\r\n");
	mySocket->CloseSocket(true);
}

/*
	@brief �����ļ���������
	@isASCII ��false��ʹ��ascii���ͣ�����ʹ��binary�������ƣ�����
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
	@brief ����"USER""PASS"ָ���¼��FTP������
	@isAnonymous �Ƿ�������¼
	@username �û���
	@password ����
*/
bool ActiveFTPClient::LoginFTPServer(bool isAnonymous)
{
	char strCommand[BUF_SIZE] = { 0 };
	char tempUsername[50] = { 0 };
	char tempPassword[50] = { 0 };
	if (isAnonymous)
	{
		// ������¼
		//printf(">>����������¼\n");
		sprintf(tempUsername, "%s", "anonymous");		
		sprintf(tempPassword, "%s", "anonymous");
	}
	else
	{
		// �û���¼
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

