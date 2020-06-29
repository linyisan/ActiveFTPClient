#include "MySocket.h"
#include <stdio.h>
#include <string>

/*
	@brief 异常错误输出
	@param message 错误信息
	@param errNo WSAGetLastError()传入的错误码
*/
void MySocket::ErrorHandle(const char * ErrorMsg, int ErrorCode)
{
	char msg[1024] = { 0 };
	if (0 != ErrorCode)
		sprintf(msg, "%serrNo:%d", ErrorMsg, ErrorCode);
	else
		strcat(msg, ErrorMsg);
	printf("%s\n", msg);
}

SOCKADDR_IN * MySocket::Bind(const char * addr_To, int port_To)
{
	int ret = 0;
	SOCKADDR_IN mAddr;
	socketServ = socket(AF_INET, SOCK_STREAM, 0);
	memset(&mAddr, 0, sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons(port_To);
	mAddr.sin_addr.S_un.S_addr = ret = inet_addr(addr_To);
	//mAddr.sin_addr.S_un.S_addr = ret = htonl(INADDR_ANY);
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
	SOCKADDR_IN name;
	memset(&name, 0, sizeof(name));
	int sz_name = sizeof(name);
	getsockname(socketServ, (struct sockaddr *)&name, &sz_name);
	msockAddr = name;
	return &name;
}

void MySocket::CloseSocket()
{
	if (socketClnt) closesocket(socketClnt);
	if (socketServ) closesocket(socketServ);
}

MySocket::MySocket()
{
	WSADATA wsaData;
	SOCKADDR_IN addr_serv;
	// 1. 打开网络库
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ErrorHandle("WSAStartup error!");
	}

	// 1.2 校验版本

	// 2. socket() 创建控制端口的套接字
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mSocket)
	{
		ErrorHandle(TEXT("socket() error"), WSAGetLastError());
		WSACleanup();
	}
}

bool MySocket::isConnected()
{
	return this->m_isConnected;
}

/*
	@brief 验证响应码
	@VerifyCode 传入测试的响应码
*/
bool MySocket::CheckResponseCode(int VerifyCode)
{
	if (VerifyCode == GetResponseCodeAtHead())
		return true;
	return false;
}

bool MySocket::Connect(const char * addr_To, int port_To)
{
	int ret = 0;
	m_isConnected = false;
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
	m_isConnected = true;
	return m_isConnected;
}

int MySocket::GetResponseCodeAtHead()
{
	char strResCode[3] = { 0 };
	memcpy(strResCode, BufForRecv, 3);
	return atoi(strResCode);
}



bool MySocket::Accept()
{
	m_isConnected = false;
	int ret = 0;
	m_isConnected = false;
	//if (!Bind(addr_me, port_me)) return false;

	// listen() 开始监听/工作
	if (SOCKET_ERROR == listen(socketServ, SOMAXCONN))
	{
		ErrorHandle("listen() socketClnt error!", WSAGetLastError());
		return false;
	}
	// accept() 接受来自FTP服务器的数据连接
	SOCKADDR_IN addr_srvData;
	int sz_addr = sizeof(addr_srvData);
	memset(&addr_srvData, 0, sz_addr);
	getsockname(socketServ, (SOCKADDR*)&msockAddr, &sz_addr);
	socketClnt = accept(socketServ, (SOCKADDR*)&addr_srvData, &sz_addr);
	if (INVALID_SOCKET == socketClnt)
	{
		ErrorHandle("accept() socketData error!", WSAGetLastError());
		return m_isConnected;
	}
	m_isConnected = true;
	return true;
}





/*
	@brief 底层发送函数
*/
bool MySocket::SendPack(const char * data, int sz_data)
{
	//FlushRecvBuf();
	if (SOCKET_ERROR == send(mSocket, data, sz_data, 0))
	{
		ErrorHandle("Send() eror!数据发送失败!\n", WSAGetLastError());
		return false;
	}
	return true;
}
/*
	@brief 原始接收数据函数
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

int MySocket::RecvPackFromClient(char *Buf)
{
	return RecvPack(Buf, socketClnt);
}
