#pragma once
#include "MySocket.h"
#define BUF_SIZE 4096
class SocketClient:public MySocket
{
public:
	SocketClient();
	bool CheckResponseCode(int VerifyCode);
	bool Connect(const char *addr_To, int port_To);
	void DisConnect();
	bool SendPack(const char *data, int sz_data);
	char* RecvPack();

protected:
	int  GetResponseCodeAtHead();
	void ErrorHandle(const char *ErrorMsg, int ErrorCode = 0);

private:
	char BufForRecv[BUF_SIZE];
	char BufForSend[BUF_SIZE];
	bool m_isConnected;
	//char *addr_Serv;
	//char *port_Serv;
	//int Code_Respond;
	SOCKET socketClnt;
};

