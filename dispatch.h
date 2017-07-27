#include "msg.h"
#include "SerMessage.h"
const int NUM_SER = 4;

class Dispatch
{
public:
    Dispatch(char* ip="127.0.0.1", int port=7000)
    {
		ser_fd = socket(AF_INET,SOCK_DGRAM,0);
		Message* msg = NULL;
		for(int i = 0; i < NUM_SER; ++i)
		{
			msg = new Message("127.0.0.1", port+=1000);
			if(i < NUM_SER/2)
				ser_msg.insert(make_pair("CENTOS", msg));
			else
			    ser_msg.insert(make_pair("UBUNTU", msg));
				
			conn_num.insert(make_pair(msg, 0));
		}
    }
	void addCliMsg(int sockfd, char* ip, int port)
	{
		Message* msg = new(ip, port);
		cli_msg.insert(make_pair(sockfd, msg));
	}
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
			else{
				strcat(res_buf, result);
				strcat(res_buf, " ");
			}

			result = strtok(NULL, delims);
		}

		res_buf[strlen(res_buf) - 1] = 0;
		return res;
	}
	
	void sendMsgToSer(char* type, char* data)
	{
		multimap<string,int>::iterator m;
        m = ser_msg[type];
		int max = 0;
		Message* msg = NULL;
		
		for(int i = 0; i < ser_msg.count(type); ++i, ++m)
		{
			if(conn_num[m->second] > max)
			{
				max = conn_num[m->second];
				msg = conn_num[m->first];
			}
		}
		
		conn_num[msg]++;
		struct sockaddr_in ser_address;
		memset(&ser_address, 0, sizeof(ser_address));
		ser_address.sin_family=AF_INET;  
		ser_address.sin_addr.s_addr=inet_addr(msg->ip);
		ser_address.sin_port=htons(msg->port);  
		sendto(ser_fd, data, strlen(data), 0, (struct sockaddr *)&ser_address,
		       sizeof(ser_address)); 
	}
private:
    multimap<char*, Message*> ser_msg;
    map<int, Message*> cli_msg;
	map<Message*, int> conn_num;
	int ser_fd; 
};
