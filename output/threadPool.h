#include "msg.h"
const int MAX_PROCESS = 1024;

class Process
{
public:
	Process();
	~Process();
private:
	pid_t m_pid;
	int m_pipe[2];
};

Process::Process():m_pid(-1){}
Process::~Process(){close(m_pid);}

template<typename T>
class ProcessPool
{
public:
	~ProcessPool();
	ProcessPool<T>* getInstance();
	void run();
	
private:
	void run_parent();
	void run_child();
	ProcessPool(int listenfd, int process_num = 8);
	
	/*进程池中最大进程数*/
	static const int MAX_PROCESS_NUMBER = 16;
	/*每个子进程能处理的最大客户数量*/
	static const int USER_PER_PEROCESS = 65536;
	/*epoll最多处理的事件数*/
	static const int MAX_EVENT_NUMBER = 10000;
	int m_listenfd;
	int m_epollfd; /*每个进程都有一个epoll内核事件表*/
	int m_index;
	int m_process_num;
	bool m_stop;
	Process *m_process;
	static ProcessPool<T>* m_instance;
};


template<typename T>
static ProcessPool::ProcessPool<T>* m_instance = NULL;

template<typename T>
ProcessPool<T>::ProcessPool(int listenfd, int process_num)
:m_process_num(process_num),m_listenfd(listenfd), m_stop(false), m_index(-1)
{
	assert(process_num > 0 && process <= MAX_PROCESS_NUMBER);
	
	m_process = new Process[m_process_num];
	m_epollfd = epoll_create(5);
	
	for(int i = 0; i < m_process_num; ++i)
	{
		int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_process[i].m_pipe);
		assert(ret == 0);
		 
		m_process[i].m_pid = fork();
		assert(m_process[i].pid != -1);
		
		if(m_process[i].pid > 0)
		{
			close(m_process[i].m_pipe[1]);
			continue;
		}
		else{
			close(m_process[i].m_pipe[0]);
			m_index = i;
			break;
		}
	}
	
}

template<typename T>
ProcessPool<T>::~ProcessPool()
{
	delete []m_instance;
}

template<typename T>
ProcessPool<T>* ProcessPool<T>::getInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new ProcessPool<T>(m_listenfd, m_process_num);
	}
	
	return m_instance;
}

static int setnonblock(int fd)
{
	int oldfd = fcntl(fd, FD_GET);
	int newfd = oldfd | O_NONBLOCK;
	
	fcntl(fd, FD_SET, newfd);
	
	return oldfd;
}

static void addfd(int epollfd, int sockfd)
{
	epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLL_IN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
	setnonblock(sockfd);
}

static void removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

template<typename T>
void ProcessPool<T>::run_child()
{
	int pipefd = m_process[m_index].m_pipe[1];
	addfd(m_epoll_fd, pipefd);
	
	int n = 0;
	int ret = -1;
	int connfd = -1;
	struct epoll_event events[MAX_EVENT_NUMBER];
	
	while(!stop)
	{
		n = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if(n < 0)
		{
			perro("epoll_wait error");
			break;
		}
		
		for(int i = 0; i < n; ++i)
		{
			int sockfd = event[i].data.fd;
			
			if(sockfd == pipefd && (events[i].events & EPOLL_IN)
			{
				int client = 0;
				
				ret = recv(sockfd, &client, sizeof(client), 0);
				if((ret < 0 && errno != EAGAIN) || ret == 0)
				{
					continue;
				}
			}
			else{
				struct sockaddr_in client_addr;
				memset(&client_addr, 0, sizeof(client_addr);
				socklen_t client_len = sizeof(client_addr);
				
				connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
				assert(connfd != -1);
				
				addfd(m_epoll_fd, connfd);
			}
		}
	}
}
template<typename T>
void ProcessPool<T>::run_parent()
{
	epoll_add(m_epoll_fd, m_listenfd);
	struct epoll_event events[MAX_EVENT_NUMBER];
	int n = 0;
	int sub_process_num = 0;
	int new_con = 1;
	
	while(!m_stop)
	{
		n = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if(n < 0)
		{
			perror("epoll_wait error");
			break;
		}
		
		for(int i = 0; i < n; ++i)
		{
			int sockfd = event[i].data.fd;
			
			if(sockfd == m_listenfd)
			{
				int j = sub_process_num;
				do{
					if(m_process[j].m_pid != -1)
						break;
					
					j = (j+1)%sub_process_num;
				}while(j != sub_process_num);
			}
			
			if(m_process[j].m_pid == -1)
			{
				m_stop = true;
				break;
			}
			
			sub_process_num = (j+1)%sub_process_num;
			send(m_process[j].m_pipe[0], (char*)&new_con, sizeof(new_con), 0);
			cout << "send requese to child " << j << endl;
		}
	}
}

template<typename T>
void ProcessPool<T>::run()
{
	if(m_index == -1)
	{
		run_parent();
	}
	else
	{
		run_child();
	}
}
