#pragma once
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 4096
class MySocket
{
public:
	MySocket();
	~MySocket();
	bool CheckResponseCode(int VerifyCode);
	bool Connect(const char *addr_To, int port_To);
	bool Accept();
	bool SendPack(const char *data, int sz_data);
	int RecvPack(char *Buf, SOCKET socket = NULL);
	int RecvPackFromClient(char *Buf);
	SOCKADDR_IN * Bind(const char *addr_To, int port_To=0);
	void CloseSocket();

protected:
	int  GetResponseCodeAtHead();
	void ErrorHandle(const char *ErrorMsg, int ErrorCode = 0);

private:
	char BufForRecv[BUF_SIZE];
	char BufForSend[BUF_SIZE];
	//char *addr_Serv;
	//char *port_Serv;
	//int Code_Respond;
	SOCKADDR_IN msockAddr;
	SOCKET mSocket, socketServ, socketClnt;
};

