#include "ActiveFTPClient.h"
void start();
void InputFromCmd(char *argv);
void Initialize();

int temp = 0;
bool isBinary = 0;
int chioce = 0;
int flag = 1;
char ip[64];
char confilm[10];
char  filePathName[64];
char  remoteFileName[64];
char oldFileName[64];
char newFileName[64];
char targetPath[64];
char  newDirectoryName[64];
char  remoteDirectoryName[64];
char  saveFileName[64];
// �˵����������Զ��庯����
void start()
{	
	Initialize();
	// 1. ǰ�᣺�����ȴ���FTP�������ӣ���¼�����ʹ������FTP����
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	// ��ʾ�û�����Ҫ���ӵ��ľ�����FTP������IP���������ӵ�����FTP������
	printf("������Ҫ���ӵ��ľ�����FTP������IP���������ӵ�����FTP����������");
	scanf("%s",ip);
	if (!(activeFTPClient->CreateFTPCtrlConnect(ip))) 
	{
		printf("IP��������\n");
		return;
	}
	// ��ʾ�û��Ƿ�������¼FTP
	while(flag)
	{
		printf("�Ƿ�Ҫ������¼FTP��yes����no����");
		scanf("%s",confilm);
		if(strcmp(confilm,"yes") == 0)
		{
			if(activeFTPClient->LoginFTPServer(true)) // user
			{
				printf("��¼�ɹ�\n");
				flag = 0;
			}
			else
			{
				printf("��¼ʧ��\n");
				return;
			}
		}
		else if(strcmp(confilm,"no") == 0)
		{
			if(activeFTPClient->LoginFTPServer(false)) // user
			{
				printf("��¼�ɹ�\n");
				flag = 0;
			}
			else
			{
				printf("��¼ʧ��\n");
				return;
			}
		}
		else
			printf("������������������\n");
			
	}
	flag = 1;
	while(flag)
	{
		Initialize();
		printf("------------------------------------------------------------------------------------------------------------------\n");
		printf("��FTP������ģʽ���ļ����䡿\n");
		printf(
			"1�������ļ���������\n2���ϴ��ļ�\n3���ļ�����\n4��ɾ���ļ�\n5���ļ�������\n6����ʾ·���µ����ļ��м����ļ���ϸ��Ϣ\n7����ʾĿǰ����·��\n8���л���������·��\n9���������ļ���\n10��ɾ���ļ��У�ֻ���ǿյģ�\n11������FTP����\n"
			  );
		printf("��ѡ��Ҫִ�еĹ��ܣ�������Ӧ�����֣���");
		rewind(stdin);
		scanf("%d",&chioce);
	// 2.2 ��ʾ�û�������������뽫Ҫ���õ�FTP���������������ö�ӦFTP����
		switch(chioce)
		{
			case 1:
				{
				printf("�������ļ��������ͣ���0��ʹ��ascii���ͣ���1��ʹ��binary���ͣ���");
				scanf("%d",&temp);
				isBinary = temp;
				if(!activeFTPClient->SetFileTransferType(isBinary))	// ascii/binary
					printf("�ļ����������Ѹı䣡\n");
				else
					printf("�ļ���������δ�ı䣡\n");
				break;
				}
			case 2:
				{
				printf("������Ҫ�ϴ������ļ���·����");
				scanf("%s",filePathName);
				if(activeFTPClient->UpdateFile(filePathName))	 // put
					printf("�ļ��ϴ��ɹ���\n");
				else
					printf("�ļ��ϴ�ʧ�ܣ�\n");
				break;
				}
			case 3:
				{
				printf("������Ҫ���ص��ļ�������չ������");
				InputFromCmd(remoteFileName);
				printf("�������ļ����浽���ص��ļ���(������·��)��");
				InputFromCmd(saveFileName);
				if(activeFTPClient->DownloadFile(remoteFileName, saveFileName))		// get
					printf("�ļ����سɹ���\n");
				else
					printf("�ļ�����ʧ�ܣ�\n");
				break;
				}
			case 4:
				{
				printf("������ɾ���ļ�������չ������");
				InputFromCmd(remoteFileName);
				if(activeFTPClient->DeleteRemoteFile(remoteFileName)) // delete
					printf("�ļ�ɾ���ɹ���\n");
				else
					printf("�ļ�ɾ��ʧ�ܣ�\n");
				break;
				}
			case 5:
				{
				printf("������Ҫ�������ľ��ļ�����");
				InputFromCmd(oldFileName);
				printf("���������ļ�����");
				InputFromCmd(newFileName);
				if(activeFTPClient->RenameRemoteFile(oldFileName, newFileName)) // rename
					printf("�ļ��������ɹ���\n");
				else
					printf("�ļ�������ʧ�ܣ�\n");
				break;
				}
			case 6:
				{
				printf("������Ҫ��ʾ��·����ֱ�ӻس���Ϊ��ǰ����·����");
				InputFromCmd(targetPath);
				activeFTPClient->ShowFTPFileDirectory(targetPath);	// dir
				break;
				}
			case 7:
				activeFTPClient->ShowFTPWorkingDirectory(); // pwd
				break;
			case 8:
				{
				printf("������Ҫ�л�����Ŀ��·����");
				InputFromCmd(targetPath);
				if(activeFTPClient->ChangeFTPWorkingDirectory(targetPath)) //cd
					printf("�л��ɹ���\n");
				else
					printf("·�������ڣ��л�ʧ�ܣ�\n");
				break;
				}
			case 9:
				{
				printf("������Ҫ���������ļ�������");
				InputFromCmd(newDirectoryName);
				if(activeFTPClient->CreateFTPDirectory(newDirectoryName)) // mkdir
					printf("�����ɹ���\n");
				else
					printf("����ʧ�ܣ�\n");
				break;
				}
			case 10:
				{
				printf("������Ҫɾ���Ŀ��ļ�������");
				InputFromCmd(remoteDirectoryName);
				if(activeFTPClient->DeleteEmptyFTPDirectory(remoteDirectoryName)) // mrdir
					printf("ɾ���ɹ���\n");
				else
					printf("ɾ��ʧ�ܣ�\n");
				break;
				}
			case 11:
				activeFTPClient->EndFTPSession(); // bye
				flag = 0;
				break;
			default:
				printf("��������ȷѡ�\n");
		}
	}
}

void InputFromCmd(char *argv)
{
	char ch = ' ';
	rewind(stdin);	// �հ׷�'\n'
	char temp[100] = { 0 };
	for(int i = 0;'\n' != (ch = getchar()) && i<sizeof(temp);i++)
	{
		temp[i] = ch;
	}
	strcpy(argv, temp);
}

void Initialize()
{
	int temp = 0;
	int chioce = 0;
	int flag = 1;
	bool isBinary = 0;
	memset(ip, 0, sizeof(ip));
	memset(confilm, 0, sizeof(confilm));
	memset(filePathName, 0, sizeof(filePathName));
	memset(remoteFileName, 0, sizeof(remoteFileName));
	memset(oldFileName, 0, sizeof(oldFileName));
	memset(newFileName, 0, sizeof(newFileName));
	memset(targetPath, 0, sizeof(targetPath));
	memset(newDirectoryName, 0, sizeof(newDirectoryName));
	memset(remoteDirectoryName, 0, sizeof(remoteDirectoryName));
	memset(saveFileName, 0, sizeof(saveFileName));
}


int main(int argc, char *argv[])
{
	start();
	system("pause");
	return 0;
}