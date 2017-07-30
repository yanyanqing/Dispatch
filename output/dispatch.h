#include "msg.h"
#include "SerMessage.h"
const int NUM_SER = 4;

class Dispatch
{
public:
    Dispatch(char* ip="127.0.0.1", int port=7000)
    {
		ser_fd = socket(AF_INET,SOCK_STREAM,0);
		Message* msg = NULL;
		for(int i = 0; i < NUM_SER; ++i)
		{
			msg = new Message(ip, port+=1000);
			conn_num.insert(make_pair(msg, 0));
			if(i < NUM_SER/2)
			{
				ser_msg.insert(make_pair((char*)("CENTOS"), msg));
			}
			else
			    ser_msg.insert(make_pair((char*)("UBUNTU"), msg));
				
			conn_num[msg]++;
		}
    }
	
   /*	void addCliMsg(int sockfd, char* ip, int port)
	{
		Message* msg = new(ip, port);
		cli_msg.insert(make_pair(sockfd, msg));
	}
   */
	char* analyzeArg(char* res_buf, char* buff)
	{
		char* res = "CENTOS";
		char delims[] = " ";
		char *result = strtok(buff, delims);
		
		while(result != NULL)
		{
			if(strcmp("CENTOS", result) == 0 || strcmp("UBUNTU", result) == 0)
			{
				res = result;
			}
			else if(strcmp("-e", result) == 0)
			{}
			else if(strstr(result, "./") != NULL)
			{
			}
			else{
				strcat(res_buf, result);
				strcat(res_buf, " ");
			}

			result = strtok(NULL, delims);
		}

		res_buf[strlen(res_buf) - 1] = 0;
		return res;
	}
	
	void sendMsgToSer(char* type, char* data, int cli_fd)
	{
		multimap<char*, Message*>::iterator m;
                m = ser_msg.find(type);
		int max = 0;
		Message* msg = NULL;
		int count = ser_msg.count(type);
		//cout << count << endl;
		for(int i = 0; i < count; ++i, ++m)
		{
			if(conn_num[m->second] > max)
			{
				max = conn_num[m->second];
				msg = m->second;
			}
		}
		
		conn_num[msg]++;
		struct sockaddr_in ser_address;
		memset(&ser_address, 0, sizeof(ser_address));
		ser_address.sin_family=AF_INET;  
		ser_address.sin_addr.s_addr=inet_addr(msg->ip);
		ser_address.sin_port=htons(msg->port); 
                ser_cli.insert(make_pair(ser_fd, cli_fd));
		socklen_t length = sizeof(ser_address);
		connect(ser_fd, (struct sockaddr *)&ser_address, length);
		send(ser_fd, data, strlen(data), 0);	
		sendMsgToCli(ser_fd, cli_fd);
	}
	void sendMsgToCli(int ser_fd, int cli_fd)
	{
		int res = 0;
		char buff[1024] = {0};
		while((res=recv(ser_fd, buff, 1024, 0)))
		{
			send(cli_fd, buff, strlen(buff), 0);
			memset(buff, 0, 1024);
		}
	}
private:
    multimap<char*, Message*> ser_msg;
    map<int, int> ser_cli;
    map<Message*, int> conn_num;
    int ser_fd; 
};

