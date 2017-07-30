#include "msg.h"
#define DISPATHCH_IP "127.0.0.1"
#define DISPATHCH_PORT 7000
#define DISPATHCH_IP "127.0.0.1"
#define DISPATHCH_PORT 7000

class Client
{
public:
	Client()
	{
		struct sockaddr_in ser;
		memset(&ser, 0, sizeof(ser));
		ser.sin_family = AF_INET;
		ser.sin_port = htons(DISPATHCH_PORT);
		ser.sin_addr.s_addr = inet_addr(DISPATHCH_IP);
		
		cli_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(-1 == cli_fd)
		{
			perror("Create socket error");
			return ;
		}
		
		if(-1==connect(cli_fd, (struct sockaddr*)&ser, sizeof(ser)))
		{
			perror("Connet error");
			return ;
		}
	}
	~Client()
	{
		close(cli_fd);
		cli_fd = -1;
	}
	void sendMessage(char **argv, int len)
	{
		char buff[256] = {0};
		for(int i = 1; i < len; ++i)
		{
			strcat(buff, argv[i]);
			strcat(buff, " ");
		}
		if(send(cli_fd, buff, strlen(buff) , 0) < 0)
		{
			perror("Send error");
			return ;
		}
	}

	void recvMessage()
	{
		int fd = -1;
		if(-1 == (fd = open("/home/yyq/output.tar.gz", O_RDONLY|O_CREAT)))
		{
			perror("open error");
			return;
		}
		char buff[1024] = {0};
		int res = 0;
		while(res=recv(cli_fd, buff, 1024, 0))
		{
			if (res < 0) perror("recv error");
			else if(res == 0) break;
			else {
				write(fd, buff, res);
			}
			memset(buff, 0, 1024);
		}
	}
private:
	int cli_fd;
};

int main(int argc, char *argv[])
{
	Client cli;
	cli.sendMessage(argv, argc);
	cli.recvMessage();

	return 0;
}

