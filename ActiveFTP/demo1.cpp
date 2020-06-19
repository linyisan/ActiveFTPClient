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
		ErrorHandle("�޷����ӵ�FTP������\n");
	else printf("�ɹ����ӷ�����\n");

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
		ErrorHandle("FTP�����ʧ��!\n", WSAGetLastError());
		return false;
	}
	return true;
}

int ControlConnect(const char *srv_addr)
{
	WSADATA wsaData;
	SOCKADDR_IN addr_serv;
	int ret;
	// 1. �������
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ErrorHandle("WSAStartup error!");
		return -1;
	}

	// 1.2 У��汾

	// 2. socket() �������ƶ˿ڵ��׽���
	socketControl = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == socketControl)
	{
		ErrorHandle(TEXT("socket() error"), WSAGetLastError());
		WSACleanup();
		return -2;
	}

	// 3. ����������Ϣ
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

	// 4. connect() ���ӵ�������
	ret = connect(socketControl, (SOCKADDR *)&addr_serv, sizeof(addr_serv));
	if (SOCKET_ERROR == ret)
	{
		ErrorHandle("connect()error!", WSAGetLastError());
		return - 3;
	}
	printf("���ӵ�%s\n", srv_addr);

	// 5.  �ж�����Ӧ����
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
			ErrorHandle("����������Ӧ����!\n");
			return -4;
		}
	}

	//�����¼��Ϣ
	// USER���� �û���
	printf("�������û�����");
	char username[30] = { 0 };
	/*scanf("%s", username);*/memcpy(username, "non", 3);
	memset(Command, 0, sizeof(Command));
	// �ϳ�/ƴ��USER����
	memcpy(Command, "USER ", strlen("USER "));
	memcpy(Command + strlen("USER "), username, strlen(username));
	memcpy(Command + strlen("USER ") + strlen(username), "\r\n", 2);
	if (!SendCommand()) return -6;

	printf("Client:%s", Command);
	// �ж�USERӦ����
	if (!RecvRespond()) return -7;
	else
	{
		if (220 == RespondCode || 331 == RespondCode)
			printf("Server:%s\n", Respond);
		else
		{
			ErrorHandle("USER��Ӧ����!\n");
			return -8;
		}
	}

	// PASS���� ����
	if (331 == RespondCode)
	{
		printf("����������:");
		char password[30] = { 0 };
		//getchar();
		/*scanf("%s", password);*/ memcpy(password, "12345679", sizeof("12345679"));

		// �ϳ�PASS����
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
		{//�ж�PASS��Ӧ��
			if (RespondCode == 230)
			{
				printf("Server:  %s\n", Respond);
			}
			else
			{
				ErrorHandle("PASS��Ӧ����\n", WSAGetLastError());
				return -11;
			}
		}
	}
	return 0;
}

/*
	@brief ����PORT������������ӣ����������������׽���socketData
*/
bool DataConnect(const char * srvAddr)
{
	// PORT���� FTP�������������ӵ��ͻ���
	memset(Command, 0, sizeof(Command));
	//�ϳ�PORT����
	memcpy(Command, "PORT ", strlen("PORT "));
	memcpy(Command + strlen("PORT "), "192,168,101,100,14,178", strlen("192,168,101,100,14,178"));//??
	memcpy(Command + strlen("PORT ") + strlen("192,168,101,100,14,178"), "\r\n", 2);
	if(!SendCommand())
		return false;
	if (!RecvRespond())
		return false;
	//�ж�PORT��Ӧ��
	else
	{
		if (200 != RespondCode)
		{
			ErrorHandle("PORT��Ӧ����");
			return false;
		}
	}

	// socket() �������������׽���
	socketClnt = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == socketClnt)
	{
		ErrorHandle("socket() socketClnt error!", WSAGetLastError());
		return false;
	}
	
	// ����ַ�˿���Ϣ
	SOCKADDR_IN addr_data;
	memset(&addr_data, 0, sizeof(addr_data));
	addr_data.sin_family = AF_INET;
	addr_data.sin_port = htons(DATAPORT);
	addr_data.sin_addr.S_un.S_addr = inet_addr(ADDR_SERV);
	// bind() �󶨶˿�
	if (SOCKET_ERROR == bind(socketClnt, (SOCKADDR*)&addr_data, sizeof(addr_data)))
	{
		ErrorHandle("bind() socketClnt error!", WSAGetLastError());
		return false;
	}

	// listen() ��ʼ����/����
	if (SOCKET_ERROR == listen(socketClnt, SOMAXCONN))
	{
		ErrorHandle("listen() socketClnt error!", WSAGetLastError());
		return false;
	}
	// accept() ��������FTP����������������
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
	printf("�ɹ�������������%d\n", DATAPORT);
	return true;
}

/*
	@brief �ײ�/ԭʼ���հ�����
	@return ���ͳɹ�ȷ��
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
	
	// ������Ӧ��
	char RelyCode[3] = { 0 };
	memcpy(RelyCode, Respond, 3);
	RespondCode = atoi(RelyCode);
	return TRUE;
}
/*
	@brief ��ȡFTP�������ļ�Ŀ¼
	
*/
bool List(const char * srvAddr)
{
	if (!DataConnect(srvAddr))
		return false;

	// �ϳ�LIST���� ��ȡĿ¼
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
			ErrorHandle("LIST��Ӧ����\n");
			return false;
		}
	}

	// ����Ŀ¼��Ϣ
	printf("Client:...\n");
	char dirinfo[BUF_SIZE] = { 0 };	// Ŀ¼����
	while (true)
	{
		char ListBuf[BUF_SIZE] = { 0 };	// ��ʱ����
		int nRecv = recv(socketData, ListBuf, sizeof(ListBuf), 0);
		if (SOCKET_ERROR == nRecv)
		{
			ErrorHandle("recv() LIST error!", WSAGetLastError());
			return false;
		}
		if (nRecv <= 0) break;
		strcat(dirinfo, ListBuf);
	}
	// ��������ȷ��״̬
	closesocket(socketData);
	if (!RecvRespond()) return false;
	else
	{
		if(226 == RespondCode)
			printf("Server:  %s\n", Respond);
		else
		{
			ErrorHandle("LIST��Ӧ����");
			return false;
		}
	}
	printf("\n%s\n", dirinfo);
	return true;
}

/*
	@brief �˳���¼����ֹUSER���������رտ�������
*/
void Quit()
{
	// �ϳ�QUIT����
	memset(Command, 0, sizeof(Command));
	memcpy(Command, "QUIT", strlen("QUIT"));
	memcpy(Command + strlen("QUIT"), "\r\n", 2);
	if (!SendCommand())
		return;
	printf("Client:  %s\n", Command);
	// ��Ӧ���ж�
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
			ErrorHandle("QUIT��Ӧ����");
			return;
		}
	}
	closesocket(socketControl);
}

/*
	@brief �쳣�������
	@param message ������Ϣ
	@param errNo WSAGetLastError()����Ĵ�����
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