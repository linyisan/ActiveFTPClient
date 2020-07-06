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
	int SendPack(const char *data, int sz_data, SOCKET socket = NULL);
	int RecvPack(char *Buf, SOCKET socket = NULL);
	int SendPackToClient(char *Buf, int sz_Buf);
	int RecvPackFromClient(char *Buf);
	SOCKADDR_IN * BindAndListen(const char *addr_To, int port_To=0);
	void CloseSocket(bool toCloseAllSocket=false);
	char *GetLocalHostIP();

protected:
	int  GetResponseCodeAtHead();
	void ErrorHandle(const char *ErrorMsg, int ErrorCode = 0);

private:
	char BufForRecv[BUF_SIZE];
	char BufForSend[BUF_SIZE];
	IN_ADDR LocalHostIP;
	SOCKADDR_IN AddrServ;
	SOCKET mSocket, socketServ, socketClnt; // 在本机充当服务器时使用socketServ,socketClnt；其中socketClnt用来收发数据
};

