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
// 菜单函数，请自定义函数名
void start()
{	
	Initialize();
	// 1. 前提：必须先创建FTP控制连接，登录后才能使用其他FTP命令
	ActiveFTPClient *activeFTPClient = new ActiveFTPClient();
	// 提示用户输入要连接到的局域网FTP服务器IP，不能连接到外网FTP服务器
	printf("请输入要连接到的局域网FTP服务器IP（不能连接到外网FTP服务器）：");
	scanf("%s",ip);
	if (!(activeFTPClient->CreateFTPCtrlConnect(ip))) 
	{
		printf("IP输入有误！\n");
		return;
	}
	// 提示用户是否匿名登录FTP
	while(flag)
	{
		printf("是否要匿名登录FTP（yes或者no）：");
		scanf("%s",confilm);
		if(strcmp(confilm,"yes") == 0)
		{
			if(activeFTPClient->LoginFTPServer(true)) // user
			{
				printf("登录成功\n");
				flag = 0;
			}
			else
			{
				printf("登录失败\n");
				return;
			}
		}
		else if(strcmp(confilm,"no") == 0)
		{
			if(activeFTPClient->LoginFTPServer(false)) // user
			{
				printf("登录成功\n");
				flag = 0;
			}
			else
			{
				printf("登录失败\n");
				return;
			}
		}
		else
			printf("输入有误，请重新输入\n");
			
	}
	flag = 1;
	while(flag)
	{
		Initialize();
		printf("------------------------------------------------------------------------------------------------------------------\n");
		printf("【FTP（主动模式）文件传输】\n");
		printf(
			"1、设置文件传输类型\n2、上传文件\n3、文件下载\n4、删除文件\n5、文件重命名\n6、显示路径下的子文件夹及其文件详细信息\n7、显示目前工作路径\n8、切换（工作）路径\n9、创建新文件夹\n10、删除文件夹（只能是空的）\n11、结束FTP连接\n"
			  );
		printf("请选择要执行的功能（输入相应的数字）：");
		rewind(stdin);
		scanf("%d",&chioce);
	// 2.2 提示用户输入命令（并输入将要调用的FTP函数参数），调用对应FTP函数
		switch(chioce)
		{
			case 1:
				{
				printf("请设置文件传输类型（若0，使用ascii类型；若1，使用binary类型）：");
				scanf("%d",&temp);
				isBinary = temp;
				if(!activeFTPClient->SetFileTransferType(isBinary))	// ascii/binary
					printf("文件传输类型已改变！\n");
				else
					printf("文件传输类型未改变！\n");
				break;
				}
			case 2:
				{
				printf("请输入要上传本地文件的路径：");
				scanf("%s",filePathName);
				if(activeFTPClient->UpdateFile(filePathName))	 // put
					printf("文件上传成功！\n");
				else
					printf("文件上传失败！\n");
				break;
				}
			case 3:
				{
				printf("请输入要下载的文件（带扩展名）：");
				InputFromCmd(remoteFileName);
				printf("请输入文件保存到本地的文件名(含绝对路径)：");
				InputFromCmd(saveFileName);
				if(activeFTPClient->DownloadFile(remoteFileName, saveFileName))		// get
					printf("文件下载成功！\n");
				else
					printf("文件下载失败！\n");
				break;
				}
			case 4:
				{
				printf("请输入删除文件（带扩展名）：");
				InputFromCmd(remoteFileName);
				if(activeFTPClient->DeleteRemoteFile(remoteFileName)) // delete
					printf("文件删除成功！\n");
				else
					printf("文件删除失败！\n");
				break;
				}
			case 5:
				{
				printf("请输入要重命名的旧文件名：");
				InputFromCmd(oldFileName);
				printf("请输入新文件名：");
				InputFromCmd(newFileName);
				if(activeFTPClient->RenameRemoteFile(oldFileName, newFileName)) // rename
					printf("文件重命名成功！\n");
				else
					printf("文件重命名失败！\n");
				break;
				}
			case 6:
				{
				printf("请输入要显示的路径，直接回车则为当前工作路径：");
				InputFromCmd(targetPath);
				activeFTPClient->ShowFTPFileDirectory(targetPath);	// dir
				break;
				}
			case 7:
				activeFTPClient->ShowFTPWorkingDirectory(); // pwd
				break;
			case 8:
				{
				printf("请输入要切换到的目标路径：");
				InputFromCmd(targetPath);
				if(activeFTPClient->ChangeFTPWorkingDirectory(targetPath)) //cd
					printf("切换成功！\n");
				else
					printf("路径不存在！切换失败！\n");
				break;
				}
			case 9:
				{
				printf("请输入要创建的新文件夹名：");
				InputFromCmd(newDirectoryName);
				if(activeFTPClient->CreateFTPDirectory(newDirectoryName)) // mkdir
					printf("创建成功！\n");
				else
					printf("创建失败！\n");
				break;
				}
			case 10:
				{
				printf("请输入要删除的空文件夹名：");
				InputFromCmd(remoteDirectoryName);
				if(activeFTPClient->DeleteEmptyFTPDirectory(remoteDirectoryName)) // mrdir
					printf("删除成功！\n");
				else
					printf("删除失败！\n");
				break;
				}
			case 11:
				activeFTPClient->EndFTPSession(); // bye
				flag = 0;
				break;
			default:
				printf("请输入正确选项！\n");
		}
	}
}

void InputFromCmd(char *argv)
{
	char ch = ' ';
	rewind(stdin);	// 空白符'\n'
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