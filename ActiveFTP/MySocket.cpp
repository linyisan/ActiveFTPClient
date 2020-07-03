#include "MySocket.h"
#include <stdio.h>
#include <string>

/*
	@brief �쳣�������
	@param message ������Ϣ
	@param errNo WSAGetLastError()����Ĵ�����
*/
void MySocket::ErrorHandle(const char * ErrorMsg, int ErrorCode)
{
	char msg[1024] = { 0 };
	if (0 != ErrorCode)
		sprintf(msg, "%serrNo:%d", ErrorMsg, ErrorCode);
	else
		strcat(msg, ErrorMsg);
	fprintf(stdout, "%s\n", msg);
	printf("����ͨ�ŷ����������������г���\n");
	system("pause");
	exit(1);
}

/*
	@brief Ϊ(�����)�׽��ְ󶨵�ַ�Ͷ˿ڣ�����ģʽ��
	@comment ��bind()�Ķ˿���0�����ͨ��getsockname()����ȡϵͳ�Զ�����Ķ˿�
*/
SOCKADDR_IN * MySocket::BindAndListen(const char * addr_To, int port_To)
{
	int ret = 0;
	SOCKADDR_IN mAddr;
	socketServ = socket(AF_INET, SOCK_STREAM, 0);
	memset(&mAddr, 0, sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons(port_To);
	mAddr.sin_addr.S_un.S_addr = ret = inet_addr(addr_To);
	if (INADDR_NONE == ret)
	{
		hostent * pHostent = gethostbyname(addr_To);
		if (pHostent)
			mAddr.sin_addr.S_un.S_addr = (*(in_addr*)pHostent->h_addr_list[0]).S_un.S_addr; ;
	}
	if (SOCKET_ERROR == bind(socketServ, (SOCKADDR*)&mAddr, sizeof(mAddr)))
	{
		ErrorHandle("Bind() error!", WSAGetLastError());
		return NULL;
	}

	// listen() ��ʼ����/����
	if (SOCKET_ERROR == listen(socketServ, 3))
	{
		ErrorHandle("listen() socketClnt error!", WSAGetLastError());
		return false;
	}

	int sz_AddrServ = sizeof(AddrServ);
	memset(&AddrServ, 0, sizeof(sz_AddrServ));
	getsockname(socketServ, (struct sockaddr *)&AddrServ, &sz_AddrServ); // ��ȡϵͳ����Ķ˿�
	return &AddrServ;
}

/*
	@brief �ر��׽���
	@toCloseAllSocket �Ƿ�ر������׽���
	@commetn ��toCloseAllSocketΪfalseʱ��ֻ�ر��������ӵ��׽��֣�����ͬʱ�ر��������ӺͿ������ӵ��׽��֡�
*/
void MySocket::CloseSocket(bool toCloseAllSocket)
{
	if (0 != socketClnt) closesocket(socketClnt);
	if (0 != socketServ) closesocket(socketServ);
	if (toCloseAllSocket)
	{
		if (0 != mSocket) closesocket(mSocket);
	}
}

/*
	@brief �ڱ����ɹ�����(connect())�������󣬻�ȡ����IP
*/
char * MySocket::GetLocalHostIP()
{
	return inet_ntoa(LocalHostIP);
}

/*
	@brief ������Ⲣ����Ĭ���׽���
*/
MySocket::MySocket()
{
	WSADATA wsaData;
	SOCKADDR_IN addr_serv;
	// 1. �������
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ErrorHandle("WSAStartup error!");
	}

	// 2. socket() �������ƶ˿ڵ��׽���
	this->mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mSocket)
	{
		ErrorHandle(TEXT("socket() error!"), WSAGetLastError());
		WSACleanup();
	}
	this->socketClnt = NULL;
	this->socketServ = NULL;
}

MySocket::~MySocket()
{
	CloseSocket(true);
	WSACleanup();
}

/*
	@brief ��֤��Ӧ��
	@VerifyCode ������Ե���Ӧ��
*/
bool MySocket::CheckResponseCode(int VerifyCode)
{
	if (VerifyCode == GetResponseCodeAtHead())
		return true;
	return false;
}

/*
	@brief ���ӵ�������
	@addr_To ������IP��ַ
	@port_To �������˿ں�
*/
bool MySocket::Connect(const char * addr_To, int port_To)
{
	int ret = 0;
	SOCKADDR_IN mAddr;
	memset(&mAddr, 0, sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons(port_To);
	mAddr.sin_addr.S_un.S_addr = ret = inet_addr(addr_To);
	if (INADDR_NONE == ret)
	{
		hostent * pHostent = gethostbyname(addr_To);
		if (pHostent)
			mAddr.sin_addr.S_un.S_addr = (*(in_addr*)pHostent->h_addr_list[0]).S_un.S_addr; ;
	}
	if (SOCKET_ERROR == connect(mSocket, (SOCKADDR*)&mAddr, sizeof(mAddr)))
	{
		ErrorHandle("connect()error!", WSAGetLastError());
		return false;
	}
	int sz_AddrServ = sizeof(AddrServ);
	getsockname(mSocket, (SOCKADDR*)&AddrServ, &sz_AddrServ); // ��ȡ����IP��ַ��Ϣ
	LocalHostIP = AddrServ.sin_addr;
	return true;
}

/*
	@brief ��ȡ3λ��Ϣ��Ӧ��
*/
int MySocket::GetResponseCodeAtHead()
{
	char strResCode[3] = { 0 };
	memcpy(strResCode, BufForRecv, 3);
	return atoi(strResCode);
}

/*
	@brief ���׽��ִ��ڼ���״̬(listen())������øú����ȴ��ͻ��˷�������
	@comment ��û�пͻ��˷������ӵ��������ú�����������
*/
bool MySocket::Accept()
{
	int ret = 0;
	
	// accept() ��������FTP����������������
	SOCKADDR_IN addr_srvData;
	int sz_addr = sizeof(addr_srvData);
	memset(&addr_srvData, 0, sz_addr);
	getsockname(socketServ, (SOCKADDR*)&addr_srvData, &sz_addr);
	memset(&addr_srvData, 0, sz_addr);
	socketClnt = accept(socketServ, (SOCKADDR*)&addr_srvData, &sz_addr);
	if (INVALID_SOCKET == socketClnt)
	{
		ErrorHandle("accept() socketData error!", WSAGetLastError());
		return false;
	}
	return true;
}

/*
	@brief �ײ㷢�ͺ���
	@data Ҫ���͵�����
	@sz_data �������ݵ��ֽڴ�С
	@socket �����͵��׽���
*/
int MySocket::SendPack(const char * data, int sz_data, SOCKET socket)
{
	int nSend = 0;
	SOCKET tempsocket = socket;
	if (0 == tempsocket)
		tempsocket = mSocket;
	nSend = send(tempsocket, data, sz_data, 0);
	if (SOCKET_ERROR == nSend)
	{
		ErrorHandle("Send() error!", WSAGetLastError());
	}
	return nSend;
}
/*
	@brief �ײ�������ݺ���
	@Buf ��Ž��յ�����
	@socket ������յ��׽���
*/
int MySocket::RecvPack(char *Buf, SOCKET socket)
{
	SOCKET tempsocket = socket;
	if (0 == tempsocket)
		tempsocket = mSocket;
	memset(BufForRecv, 0, sizeof(BufForRecv));
	int nRecv = recv(tempsocket, BufForRecv, sizeof(BufForRecv), 0);
	if (SOCKET_ERROR == nRecv)
	{
		ErrorHandle("RecvPack() error!", WSAGetLastError());
		return false;
	}
	memcpy(Buf, BufForRecv, nRecv);
	return nRecv;
}

/*
	@brief ��������(�����䵱�����ʱ)
	@sz_Buf Ҫ�������ݵ��ֽڴ�С
	@return �ɹ�/ʵ�ʷ������ݵ��ֽڴ�С
*/
int MySocket::SendPackToClient(char *Buf, int sz_Buf)
{
	return SendPack(Buf,sz_Buf, socketClnt);
}

/*
	@brief ��������(�����䵱�����ʱ)
	@return �ɹ�/ʵ�ʽ������ݵ��ֽڴ�С
*/
int MySocket::RecvPackFromClient(char *Buf)
{
	return RecvPack(Buf, socketClnt);
}
