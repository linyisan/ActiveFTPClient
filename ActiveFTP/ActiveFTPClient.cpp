#include "ActiveFTPClient.h"
#include <stdio.h>
#include <conio.h>

ActiveFTPClient::ActiveFTPClient()
{
	mySocket = new MySocket();
}

/*
	@breif �������������Ӧ��Ϣ
	@comment ���͵������ֽڴ�С��strlen(cmd)������sizeof(cmd)
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
	@brief ��������ͨ��������ģʽ��
	@comment ��ͨ��Bind()ϵͳ�Զ�����˿ںţ�Ȼ�󹹽�������"PORT"(��"PORT 192,168,101,100,14,178")ָ���֪FTP��������
	������Accept()�ȴ�FTP���������ӵ����ض˿�(14*256+178)
*/
bool ActiveFTPClient::CreateFTPDataConnect()
{
	char *strResMsg;
	char strCommand[BUF_SIZE] = { 0 };

	SOCKADDR_IN addr_data = *mySocket->BindAndListen(mySocket->GetLocalHostIP());
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

	//if (!mySocket->Accept()) return false;
	//printf(">>�ɹ�������������%d\n", tempPort);
	return true;

}

/*
	@brief �ַ����滻
	@sSrc ���滻��ԭ�ַ���
	@sMatchStr Ҫƥ����ַ���
	@sReplaceStr ��ƥ����ַ����滻�����ַ���
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
	@brief ��������ͨ��
	@addr_To FTP������IP
	@port_To FTP�������˿�
	@isAnonymous �Ƿ�������¼
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
		if (!LoginFTPServer("anonymous", TEXT(""))) return false;	// ������¼
	}
	else
		{
			printf("�������û���:");
			char username[30];
			memset(username, 0, sizeof(username));
			scanf("%s", username);
			printf("����������:");
			char password[30];
			memset(password, 0, sizeof(password));

			char ch;
			ch = getchar();//���������û���ʱ��\n
			while (true)
			{
				ch = getch();
				//putchar(ch);
				if (ch == '\r') break;
				//if (ch=='\n') break;

				password[strlen(password)] = ch;
			}

			if (!LoginFTPServer(username, password)) return false;	// �û���¼
		}
	return true;
}

/*
	@brief ��ȡĿ¼
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
	if (!((mySocket->CheckResponseCode(125))
			|| (mySocket->CheckResponseCode(150)))) return false;

	if (!mySocket->Accept()) return false;
	printf(">>�ɹ�������������\n");

	printf(">>���ڻ�ȡĿ¼:....\n");
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
	@bref �ļ�����
	@remoteFileName Ҫ���ص��ļ���
	@saveFileName �ļ����浽���ص��ļ���(��ú�����·��)
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


	if (!mySocket->Accept()) return false;
	printf(">>�ɹ�������������\n");

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
	printf(">>�ɹ������ļ�%s", remoteFileName);
	//if (!mySocket->CheckResponseCode(226)) return false;
	mySocket->CloseSocket();
	return true;
}

/*
	@brief �ϴ��ļ�
	@fileName Ҫ�ϴ������ļ���·��
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
	printf(">>�ɹ��ϴ��ļ�%s", fileName);
	mySocket->CloseSocket();
	return true;
}

/*
	@brief ����"USER""PASS"ָ���¼��FTP������
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

