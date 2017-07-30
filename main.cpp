#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <dirent.h>
using namespace std;

int main()
{
    cout << "program start !" << endl;
    pid_t pid = fork();
    char buff[] = {"123"};
    int aa;
    cout << atoi(buff) << endl;
    if(pid == -1)
    {
        perror("fork error");
    }
    else if(pid == 0)
    {
        char buff[] = {"g++ main.cpp -o main"};
	system("mkdir output");
        system(buff);
        system("cp * output");
        cout << "ouuu" << endl;
    }
    else
{
       int stat;
       waitpid(-1, &stat, WNOHANG);
       cout << "father" << endl;
}     

    return 0;
}
/*
int main(void)
{
    int epfd,nfds;
    struct epoll_event ev,events[5];                    //ev¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
    epfd = epoll_create(1);                                //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN|EPOLLET;                        //¿¿¿¿¿¿¿¿¿ET¿¿
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);    //¿¿epoll¿¿
    for(;;)
    {
        nfds = epoll_wait(epfd, events, 5, -1);
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd==STDIN_FILENO)
            {
                printf("welcome to epoll's word!\n");
                ev.data.fd = STDIN_FILENO;
                ev.events = EPOLLIN|EPOLLET;                        //¿¿ET¿¿
                epoll_ctl(epfd, EPOLL_CTL_MOD, STDIN_FILENO, &ev);    //¿¿epoll¿¿¿ADD¿¿¿
            }            
        }
    }
}

*/

/*#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

int main(void)
{
    int epfd,nfds;
    struct epoll_event ev,events[5];                    //ev¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
    epfd = epoll_create(1);                                //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
    ev.data.fd = STDOUT_FILENO;
    ev.events = EPOLLOUT|EPOLLET;                        //¿¿¿¿¿¿¿¿¿ET¿¿
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDOUT_FILENO, &ev);    //¿¿epoll¿¿
    for(;;)
    {
        nfds = epoll_wait(epfd, events, 5, -1);
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd==STDOUT_FILENO)
            {
                printf("welcome to epoll's word!\n");
            }            
        }
    }
}

*/
/*
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>

using namespace std;

int main(void)
{
    int epfd,nfds;
    struct epoll_event ev,events[5];                    //ev¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
    epfd = epoll_create(1);                                //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN;                        //¿¿¿¿¿¿¿¿¿ET¿¿
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);    //¿¿epoll¿¿
    for(;;)
    {
        nfds = epoll_wait(epfd, events, 5, -1);
        for(int i = 0; i < nfds; i++)
        {
	    cout << nfds << endl;
            if(events[i].data.fd==STDIN_FILENO)
                printf("Something happened with stdin!\n");
        }
    }
}*/
