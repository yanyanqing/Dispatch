#include "msg.h"
#include "dispatch.h"
const int MAX_PROCESS = 1024;

class Process
{
public:
	Process();
	~Process();
public:
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
	static ProcessPool<T>* getInstance(int listenfd = -1, int process_num = 3);
	void run();
	
private:
    void setup_sig_pipe();
	void run_parent();
	void run_child();
    ProcessPool(int listenfd = -1, int process_num = 3);
	
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
    T *m_load;
	static ProcessPool<T>* m_instance;
};

static int sig_pipe[2];

template<typename T>
ProcessPool<T>* ProcessPool<T>:: m_instance = NULL;



template<typename T>
ProcessPool<T>::ProcessPool(int listenfd, int process_num)
:m_process_num(process_num),m_listenfd(listenfd), m_stop(false), m_index(-1)
{
	assert(process_num > 0 && process_num <= MAX_PROCESS_NUMBER);
	
    m_dispatch = new T();
    
	
	m_process = new Process[m_process_num];
	m_epollfd = epoll_create(5);
	
	for(int i = 0; i < m_process_num; ++i)
	{
		int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_process[i].m_pipe);
		assert(ret == 0);
		
		m_process[i].m_pid = fork();
		assert(m_process[i].m_pid != -1);
		 
		if(m_process[i].m_pid > 0)
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
    delete m_load;
}


template<typename T>
ProcessPool<T>* ProcessPool<T>::getInstance(int listenfd, int process_num)
{
	if(m_instance == NULL)
	{
		m_instance = new ProcessPool<T>(listenfd, process_num);
	}
	
	return m_instance;
}

static int setnonblock(int fd)
{
	int oldfd = fcntl(fd, F_GETFL);
	int newfd = oldfd | O_NONBLOCK;
	
	fcntl(fd, F_SETFL, newfd);
	
	return oldfd;
}

static void addfd(int epollfd, int sockfd)
{
	epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
	setnonblock(sockfd);
}

static void removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

static void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipe[1], (char*)&msg, 1, 0);
}

static void addsig(int sig, void(handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    if(restart)
    {
       sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

template<typename T>
void ProcessPool<T>::setup_sig_pipe()
{
    setnonblock(sig_pipe[1]);
    addfd(m_epollfd, sig_pipe[0]);

    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT,  sig_handler);
    addsig(SIGPIPE, SIG_IGN);
}

template<typename T>
void ProcessPool<T>::run_child()
{
    setup_sig_pipe();
	int pipefd = m_process[m_index].m_pipe[1];
	addfd(m_epollfd, pipefd);
	
	int n = 0;
	int ret = -1;
	int m_connfd = -1;
	struct epoll_event events[MAX_EVENT_NUMBER];
	
	while(!m_stop)
	{
		n = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if(n < 0)
		{
			perror("epoll_wait error");
			break;
		}
		
		for(int i = 0; i < n; ++i)
		{
			int sockfd = events[i].data.fd;
			if(sockfd == pipefd && (events[i].events & EPOLLIN))
			{
				int client = 0;
				ret = recv(sockfd, (char*)&client, sizeof(client), 0);
				if((ret < 0 && errno != EAGAIN) || ret == 0)
				{
					continue;
				}
			}
			else if(sockfd == m_listenfd)
            {
			    struct sockaddr_in client_addr;
				memset(&client_addr, 0, sizeof(client_addr));
				socklen_t client_len = sizeof(client_addr);
				
				m_connfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
                cout << "ip:" << inet_ntoa(client_addr.sin_addr) << " port:" << ntohs(client_addr.sin_port) << endl;
				addfd(m_epollfd, m_connfd);
				m_dispatch->addCliMsg(sockfd, inet_ntoa(client_addr.sin_addr),
				 ntohs(client_addr.sin_port));
			}
            else if(sockfd == sig_pipe[0] && events[i].events & EPOLLIN)
            {
                int sig;
                char signals[1024];
                int ret = recv(sig_pipe[0], signals, sizeof(signals), 0);
                if(ret <= 0)
                    continue;

                else{
                    for(int i = 0; i < ret; ++i)
                    {
                        switch(signals[i])
                        {
                        case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                                    continue;
                                break;
                            }
                        case SIGTERM:
                        case SIGINT:
                            {
                                m_stop = true;
                                break;
                            }
                        default:
                            continue;
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                char buff[128] = {0};
				char res[128] = {0};
		
                int ret = recv(sockfd, buff, 128, 0);
				char* type = m_dispatch->analyzeArg(res, buff);				
                m_dispatch->sendMsgToSer(type, res);
            }
            else if(events[i].events & EPOLLOUT)
            {
                continue;
            }
            else {}
		}
	}

    close(pipefd);
    close(m_epollfd);
}

template<typename T>
void ProcessPool<T>::run_parent()
{
    setup_sig_pipe();
	addfd(m_epollfd, m_listenfd);
	struct epoll_event events[MAX_EVENT_NUMBER];
	int n = 0;
	int sub_process_num = 0;
	int new_con = 1;
	
	while(!m_stop)
	{
		n = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if(n < 0)
		{
			perror("epoll_wait error");
			break;
		}
		for(int i = 0; i < n; ++i)
		{
			int sockfd = events[i].data.fd;
			
			if(sockfd == m_listenfd)
			{
				int j = sub_process_num;
				do{
					if(m_process[j].m_pid != -1)
						break;
					
					j = (j+1)%m_process_num;
				}while(j != sub_process_num);
			
			
			    if(m_process[j].m_pid == -1)
		    	{
			    	m_stop = true;
				    break;
		    	}
			    
		    	sub_process_num = (j+1)%m_process_num;
			    send(m_process[j].m_pipe[0], (char*)&new_con, sizeof(new_con), 0);
			    cout << "send requese to child " << j << endl;
		    }
            else if(sockfd == sig_pipe[0] && events[i].events & EPOLLIN)
            {
                int sig;
                char signals[1024];
                int ret = recv(sig_pipe[0], signals, sizeof(signals), 0);
                if(ret <= 0)
                    continue;
                else
                {
                    for(int i = 0; i < ret; ++i)
                    {
                        switch(signals[i])
                        {
                        case SIGCHLD:
                            {
                                int stat;
                                pid_t pid;
                                while((pid=waitpid(-1, &stat, WNOHANG)) > 0)
                                {
                                    for(int i = 0; i < m_process_num; ++i)
                                    {
                                        if(m_process[i].m_pid == pid)
                                        {
                                            close(m_process[i].m_pipe[0]);
                                            m_process[i].m_pid = -1;
                                        }
                                    }
                                }

                                m_stop = true;
                                for(int i = 0; i < m_process_num; ++i)
                                {
                                    if(m_process[i].m_pid != -1)
                                    {
                                        m_stop = false;
                                    }
                                }
                                break;
                            }
                        case SIGTERM:
                        case SIGINT:
                            {
                                for(int i = 0; i < m_process_num; ++i)
                                {
                                    int pid = m_process[i].m_pid;
                                    if(pid != -1)
                                    {
                                        kill(pid, SIGTERM);
                                    }
                                }

                                break;
                            }
                        default: break;
                        }
                    }
                }
            }
            else{}
	    }
    }

    close(m_epollfd);
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

int main()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd != -1);
	
    struct sockaddr_in ser;
    memset(&ser, 0, sizeof(ser));

    ser.sin_family = AF_INET;
    ser.sin_port = htons(7000);
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    assert(bind(listenfd, (sockaddr*)&ser, sizeof(ser)) != -1);

    assert(listen(listenfd, 5) != 1);

    ProcessPool<LoadBalanced> *pool = ProcessPool<LoadBalanced>::getInstance(listenfd);
    
    if(pool != NULL)
    {
        pool->run();
        delete pool;
    }

    close(listenfd);

    return 0;
}

