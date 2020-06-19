#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 4096
#define CTRLPORT 21
#define DATAPORT 3762
#define ADDR_SERV "99.84.224.117" //ftp.mozilla.org

SOCKET socketControl, socketData, socketClnt;
char Respond[BUF_SIZE];
char Command[BUF_SIZE];
int RespondCode;
BOOL WINAPI fun(DWORD dwCtrlType);
bool SendCommand();
int ControlConnect(const char *srv_addr);
bool DataConnect(const char * srvAddr);
bool RecvRespond();
bool List(const char *srvAddr);
void Quit();
void ErrorHandle(const char *message, int errNo = 0);

int main(int argc, char *argv[])
{
	SetConsoleCtrlHandler(fun, TRUE);
	if (0 > ControlConnect(ADDR_SERV))
		ErrorHandle("无法连接到FTP服务器\n");
	else printf("成功连接服务器\n");

	List(ADDR_SERV);
	Quit();
	return 0;
}

BOOL __stdcall fun(DWORD dwCtrlType)
{
	if (socketControl) closesocket(socketControl);
	if (socketData) closesocket(socketData);
	if (socketClnt) closesocket(socketData);
	WSACleanup();
	return TRUE;
}

bool SendCommand()
{
	if (SOCKET_ERROR == send(socketControl, Command, strlen(Command), 0))
	{
		ErrorHandle("FTP命令发送失败!\n", WSAGetLastError());
		return false;
	}
	return true;
}

int ControlConnect(const char *srv_addr)
{
	WSADATA wsaData;
	SOCKADDR_IN addr_serv;
	int ret;
	// 1. 打开网络库
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ErrorHandle("WSAStartup error!");
		return -1;
	}

	// 1.2 校验版本

	// 2. socket() 创建控制端口的套接字
	socketControl = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == socketControl)
	{
		ErrorHandle(TEXT("socket() error"), WSAGetLastError());
		WSACleanup();
		return -2;
	}

	// 3. 填充服务器信息
	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(CTRLPORT);
	addr_serv.sin_addr.S_un.S_addr = ret = inet_addr(srv_addr);
	if (INADDR_NONE == ret)
	{
		hostent * pHostent = gethostbyname(srv_addr);
		if(pHostent)
			addr_serv.sin_addr.S_un.S_addr = (*(in_addr*)pHostent->h_addr_list[0]).S_un.S_addr;
	}

	// 4. connect() 连接到服务器
	ret = connect(socketControl, (SOCKADDR *)&addr_serv, sizeof(addr_serv));
	if (SOCKET_ERROR == ret)
	{
		ErrorHandle("connect()error!", WSAGetLastError());
		return - 3;
	}
	printf("连接到%s\n", srv_addr);

	// 5.  判断连接应答码
	if (!RecvRespond())
		return -4;
	else
	{
		if (220 == RespondCode)
		{
			printf("Server:%s\n", Respond);
		}
		else
		{
			ErrorHandle("控制连接响应错误!\n");
			return -4;
		}
	}

	//构造登录信息
	// USER命令 用户名
	printf("请输入用户名：");
	char username[30] = { 0 };
	/*scanf("%s", username);*/memcpy(username, "non", 3);
	memset(Command, 0, sizeof(Command));
	// 合成/拼凑USER命令
	memcpy(Command, "USER ", strlen("USER "));
	memcpy(Command + strlen("USER "), username, strlen(username));
	memcpy(Command + strlen("USER ") + strlen(username), "\r\n", 2);
	if (!SendCommand()) return -6;

	printf("Client:%s", Command);
	// 判断USER应答码
	if (!RecvRespond()) return -7;
	else
	{
		if (220 == RespondCode || 331 == RespondCode)
			printf("Server:%s\n", Respond);
		else
		{
			ErrorHandle("USER响应错误!\n");
			return -8;
		}
	}

	// PASS命令 密码
	if (331 == RespondCode)
	{
		printf("请输入密码:");
		char password[30] = { 0 };
		//getchar();
		/*scanf("%s", password);*/ memcpy(password, "12345679", sizeof("12345679"));

		// 合成PASS命令
		memset(Command, 0, sizeof(Command));
		memcpy(Command, "PASS ", strlen("PASS "));
		memcpy(Command + strlen("PASS "), password, strlen(password));
		memcpy(Command + strlen("PASS ") + strlen(password), "\r\n", 2);
		if (!SendCommand())
			return -9;

		printf("Client: PASS ******\n");
		if (!RecvRespond())
			return -10;
		else
		{//判断PASS响应码
			if (RespondCode == 230)
			{
				printf("Server:  %s\n", Respond);
			}
			else
			{
				ErrorHandle("PASS响应错误！\n", WSAGetLastError());
				return -11;
			}
		}
	}
	return 0;
}

/*
	@brief 发送PORT命令发起主动连接，并建立数据连接套接字socketData
*/
bool DataConnect(const char * srvAddr)
{
	// PORT命令 FTP服务器主动连接到客户端
	memset(Command, 0, sizeof(Command));
	//合成PORT命令
	memcpy(Command, "PORT ", strlen("PORT "));
	memcpy(Command + strlen("PORT "), "192,168,101,100,14,178", strlen("192,168,101,100,14,178"));//??
	memcpy(Command + strlen("PORT ") + strlen("192,168,101,100,14,178"), "\r\n", 2);
	if(!SendCommand())
		return false;
	if (!RecvRespond())
		return false;
	//判断PORT响应码
	else
	{
		if (200 != RespondCode)
		{
			ErrorHandle("PORT响应错误");
			return false;
		}
	}

	// socket() 创建数据连接套接字
	socketClnt = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == socketClnt)
	{
		ErrorHandle("socket() socketClnt error!", WSAGetLastError());
		return false;
	}
	
	// 填充地址端口信息
	SOCKADDR_IN addr_data;
	memset(&addr_data, 0, sizeof(addr_data));
	addr_data.sin_family = AF_INET;
	addr_data.sin_port = htons(DATAPORT);
	addr_data.sin_addr.S_un.S_addr = inet_addr(ADDR_SERV);
	// bind() 绑定端口
	if (SOCKET_ERROR == bind(socketClnt, (SOCKADDR*)&addr_data, sizeof(addr_data)))
	{
		ErrorHandle("bind() socketClnt error!", WSAGetLastError());
		return false;
	}

	// listen() 开始监听/工作
	if (SOCKET_ERROR == listen(socketClnt, SOMAXCONN))
	{
		ErrorHandle("listen() socketClnt error!", WSAGetLastError());
		return false;
	}
	// accept() 接受来自FTP服务器的数据连接
	SOCKADDR_IN addr_srvData;
	int sz_addr = sizeof(addr_srvData);
	memset(&addr_srvData, 0, sz_addr);
	socketData = accept(socketClnt, (SOCKADDR*)&addr_srvData, &sz_addr);
	if (INVALID_SOCKET == socketData)
	{
		ErrorHandle("accept() socketData error!", WSAGetLastError());
		return false;
	}
	// 
	printf("成功建立数据连接%d\n", DATAPORT);
	return true;
}

/*
	@brief 底层/原始接收包函数
	@return 发送成功确认
*/
bool RecvRespond()
{
	int ret;
	memset(Respond, 0, BUF_SIZE);
	ret = recv(socketControl, Respond, sizeof(Respond), 0);
	if (SOCKET_ERROR == ret)
	{
		ErrorHandle("recv() socketControl error!", WSAGetLastError());
		return FALSE;
	}
	
	// 解析响应码
	char RelyCode[3] = { 0 };
	memcpy(RelyCode, Respond, 3);
	RespondCode = atoi(RelyCode);
	return TRUE;
}
/*
	@brief 获取FTP服务器文件目录
	
*/
bool List(const char * srvAddr)
{
	if (!DataConnect(srvAddr))
		return false;

	// 合成LIST命令 获取目录
	memcpy(Command, "LIST", strlen("LIST"));
	memcpy(Command + strlen("LIST"), "\r\n", 2);
	if (!SendCommand())
		return false;
	printf("Client:  %s\n", Command);
	if (!RecvRespond())
		return false;
	else
	{
		if (125 == RespondCode || 150 == RespondCode || 226 == RespondCode)
		{
			printf("Server:  %s\n", Respond);
		}
		else
		{
			ErrorHandle("LIST响应错误！\n");
			return false;
		}
	}

	// 接收目录信息
	printf("Client:...\n");
	char dirinfo[BUF_SIZE] = { 0 };	// 目录缓存
	while (true)
	{
		char ListBuf[BUF_SIZE] = { 0 };	// 临时缓存
		int nRecv = recv(socketData, ListBuf, sizeof(ListBuf), 0);
		if (SOCKET_ERROR == nRecv)
		{
			ErrorHandle("recv() LIST error!", WSAGetLastError());
			return false;
		}
		if (nRecv <= 0) break;
		strcat(dirinfo, ListBuf);
	}
	// 控制连接确认状态
	closesocket(socketData);
	if (!RecvRespond()) return false;
	else
	{
		if(226 == RespondCode)
			printf("Server:  %s\n", Respond);
		else
		{
			ErrorHandle("LIST响应错误！");
			return false;
		}
	}
	printf("\n%s\n", dirinfo);
	return true;
}

/*
	@brief 退出登录。终止USER，服务器关闭控制连接
*/
void Quit()
{
	// 合成QUIT命令
	memset(Command, 0, sizeof(Command));
	memcpy(Command, "QUIT", strlen("QUIT"));
	memcpy(Command + strlen("QUIT"), "\r\n", 2);
	if (!SendCommand())
		return;
	printf("Client:  %s\n", Command);
	// 响应码判断
	if (!RecvRespond())
		return;
	else
	{
		if (RespondCode == 221)
		{
			printf("Server:  %s\n", Respond);
		}
		else
		{
			ErrorHandle("QUIT响应错误！");
			return;
		}
	}
	closesocket(socketControl);
}

/*
	@brief 异常错误输出
	@param message 错误信息
	@param errNo WSAGetLastError()传入的错误码
*/
void ErrorHandle(const char *message, int errNo)
{
	char msg[1024] = { 0 };
	if (0 != errNo)
		sprintf(msg, "%serrNo:%d", message, errNo);
	else
		strcat(msg, message);
	printf("%s\n", msg);
	exit(1);
}