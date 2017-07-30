#include "msg.h"
struct Message
{
    char ip[256];
    int port;
    char env[256];
    Message(char *_ip="127.0.0.1", int _port=8000)
    {
        strcpy(ip, _ip);
	port = _port;
    }
};
