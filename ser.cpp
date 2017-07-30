#include "msg.h"
#include<iostream>
using namespace std;
class Server
{
public:
	Server(string ip = "127.0.0.1", int port = 8000);
	~Server();
	
	void sendMsg();
	void run();
	void run_child(int fd);
private:
	int m_listenfd;
	int m_acceptfd;
	
};

Server::Server(string ip, int port)
{
	m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(m_listenfd != -1);
	
	struct sockaddr_in ser;
	memset(&ser, 0, sizeof(ser));
	
	ser.sin_family = AF_INET;
	ser.sin_port = htons(port);
	ser.sin_addr.s_addr = inet_addr(ip.c_str());
	
	assert(bind(m_listenfd, (sockaddr*)&ser, sizeof(ser)) != -1);
	
	assert(listen(m_listenfd, 5) != 1);
}

Server::~Server(){close(m_listenfd);}

void Server::run()
{
	struct sockaddr_in load_addr;
	memset(&load_addr, 0, sizeof(load_addr));
	
	socklen_t len = sizeof(load_addr);
	while(1)
	{
		m_acceptfd = accept(m_listenfd, (struct sockaddr*)&load_addr, &len);
		assert(m_acceptfd != -1);
		
		cout << "Dispatch connect success" << " ip:" << inet_ntoa(load_addr.sin_addr)
		<< " port:" << ntohs(load_addr.sin_port) << endl;

		pid_t pid = fork();
		
		if(pid < 0)
		{
			perror("fork error");
			return ;
		}
		if(pid == 0)
		{
			cout << "run_child()" << endl;
			run_child(m_acceptfd);
		}
		else
		{
			int stat;
			waitpid(-1, &stat, WNOHANG);
		}
	}
	
}

void Server::run_child(int fd)
{
	char buff[1024] = {0};
	int res = recv(fd, buff, 1024, 0);
	cout << buff << endl;
	if(res < 0)
	{
        	cout << "recv error";
		exit(0);
        }
   
	else
	{
		//char fds[4] = {0};
		//strcpy(buff+strlen(buff), fds);
		//int outfd = atoi(fds);
		system("mkdir output");
		system(buff);
		system("cp * output");
		system("tar zcvf output.tar.gz /home/yyq/2017/Dispatch/output");
		
		off_t res = 0;
		char data[1024] = {0};
		int infd = open("output.tar.gz", O_RDONLY);
		res = read(infd, data, 1024);
		while((res = read(infd, data, 1024)))
		{
		    send(fd, data, strlen(data), 0);
		    memset(buff, 0, 1024);
		}
		
	}
    
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        cout << "invalid_argument! " << endl;
        exit(0);
    }
    int port = atoi(argv[1]);

    Server ser("127.0.0.1", port);
    ser.run();
	
    return 0;
}
