#include "SocketClient.h"
#include <stdio.h>
#include <string>


SocketClient::SocketClient()
{
}


bool SocketClient::CheckResponseCode(int VerifyCode)
{
	return MySocket::CheckResponseCode( VerifyCode);
}

bool SocketClient::Connect(const char * addr_To, int port_To)

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
	if (SOCKET_ERROR == connect(socketClnt, (SOCKADDR*)&mAddr, sizeof(mAddr)))
	{
		ErrorHandle("connect()error!", WSAGetLastError());
		return false;
	}
	m_isConnected = true;
	return m_isConnected;
}

void SocketClient::DisConnect()
{
}

bool SocketClient::SendPack(const char * data, int sz_data)
{
	return MySocket::SendPack(data, sz_data);
}

char * SocketClient::RecvPack()
{
	//return MySocket::RecvPack();
	char str;
	return &str;
}

int SocketClient::GetResponseCodeAtHead()
{
	return MySocket::GetResponseCodeAtHead();
}

void SocketClient::ErrorHandle(const char * ErrorMsg, int ErrorCode)
{
	MySocket::ErrorHandle(ErrorMsg, ErrorCode);
}
