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
	fprintf(stdout, "%s\n", msg);
	printf("网络通信发生错误，请重新运行程序！\n");
	system("pause");
	exit(1);
}

/*
	@brief 为(服务端)套接字绑定地址和端口（主动模式）
	@comment 如bind()的端口填0，则可通过getsockname()来获取系统自动分配的端口
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

	// listen() 开始监听/工作
	if (SOCKET_ERROR == listen(socketServ, 3))
	{
		ErrorHandle("listen() socketClnt error!", WSAGetLastError());
		return false;
	}

	int sz_AddrServ = sizeof(AddrServ);
	memset(&AddrServ, 0, sizeof(sz_AddrServ));
	getsockname(socketServ, (struct sockaddr *)&AddrServ, &sz_AddrServ); // 获取系统分配的端口
	return &AddrServ;
}

/*
	@brief 关闭套接字
	@toCloseAllSocket 是否关闭所有套接字
	@commetn 当toCloseAllSocket为false时，只关闭数据连接的套接字，否则同时关闭数据连接和控制连接的套接字。
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
	@brief 在本机成功连接(connect())服务器后，获取本机IP
*/
char * MySocket::GetLocalHostIP()
{
	return inet_ntoa(LocalHostIP);
}

/*
	@brief 打开网络库并创建默认套接字
*/
MySocket::MySocket()
{
	WSADATA wsaData;
	SOCKADDR_IN addr_serv;
	// 1. 打开网络库
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ErrorHandle("WSAStartup error!");
	}

	// 2. socket() 创建控制端口的套接字
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
	@brief 验证响应码
	@VerifyCode 传入测试的响应码
*/
bool MySocket::CheckResponseCode(int VerifyCode)
{
	if (VerifyCode == GetResponseCodeAtHead())
		return true;
	return false;
}

/*
	@brief 连接到服务器
	@addr_To 服务器IP地址
	@port_To 服务器端口号
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
	getsockname(mSocket, (SOCKADDR*)&AddrServ, &sz_AddrServ); // 获取本机IP地址信息
	LocalHostIP = AddrServ.sin_addr;
	return true;
}

/*
	@brief 获取3位消息响应码
*/
int MySocket::GetResponseCodeAtHead()
{
	char strResCode[3] = { 0 };
	memcpy(strResCode, BufForRecv, 3);
	return atoi(strResCode);
}

/*
	@brief 在套接字处于监听状态(listen())，后调用该函数等待客户端发起连接
	@comment 若没有客户端发起连接到本机，该函数是阻塞的
*/
bool MySocket::Accept()
{
	int ret = 0;
	
	// accept() 接受来自FTP服务器的数据连接
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
	@brief 底层发送函数
	@data 要发送的内容
	@sz_data 发送内容的字节大小
	@socket 负责发送的套接字
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
	@brief 底层接收数据函数
	@Buf 存放接收的内容
	@socket 负责接收的套接字
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
	@brief 发送数据(本机充当服务端时)
	@sz_Buf 要发送数据的字节大小
	@return 成功/实际发送数据的字节大小
*/
int MySocket::SendPackToClient(char *Buf, int sz_Buf)
{
	return SendPack(Buf,sz_Buf, socketClnt);
}

/*
	@brief 接收数据(本机充当服务端时)
	@return 成功/实际接收数据的字节大小
*/
int MySocket::RecvPackFromClient(char *Buf)
{
	return RecvPack(Buf, socketClnt);
}
