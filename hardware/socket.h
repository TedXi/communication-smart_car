#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "../public.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>  
#include <time.h> 

#define NETWORK_DELAY_TIME 25*1000
class NetWork_Client
{
public:
    NetWork_Client(char* addr,unsigned int port,char local_id_in);
	~NetWork_Client();
	
	unsigned int get_server_int32_addr();
	unsigned int get_server_port();
	
	unsigned int socket_connect();

	int send_data(void *data, unsigned int lenth);
	int recv_data(char *buff, int char_num);
    int recv_data_block(char *buff, int char_num);
    int recv_data_block(char *buff, int char_num, unsigned int timeout);

    char local_id;
    char server_id;
private:
	unsigned int ip_addr2int32(char* addr_input);
	
	unsigned char flage_connected;
    char* server_addr;
    unsigned int server_addr_int32;
    unsigned int server_port;

    int socket_fd;
    fd_set fdset;
    struct sockaddr_in addr_ser;
};

/******************************************************
example:
int main(){
	NetWork_Client testnet((char*)"192.168.1.249",8000);
	testnet.socket_connect();
	testnet.send_data((char *)"123\n",5);
	while(1){
		char *recv_buff = (char *)calloc(1,100);
		if(testnet.recv_data(recv_buff，100) > 0){
			printf("%s",recv_buff);
		}
		free(recv_buff);
	}
}
******************************************************/
struct Connect_fd
{
    int fd;
    char sour_ID;
};

class NetWork_Server
{
public:
    NetWork_Server(int port, int max_listen_num, char local_id_in);
    ~NetWork_Server();
    int send_data(void *data, unsigned int lenth,char send_id);
    int recv_data_block(char *buff, int char_num, char recv_id);
    void witch_connect_readable(vector<Connect_fd> &fdin);
    void get_connects(vector<Connect_fd> &fdin);

    vector<Connect_fd> connect_fd;
    char local_id;
private:
    unsigned int ip_addr2int32(char* addr_input);
    void get_new_connection();

    thread *Accept_Thread;
    mutex g_lock;

    struct sockaddr_in myaddr;
    int dev_ip_int = 0;
    int listen_port = 0;
    int max_client;
    int server_sock_fd;
    int accept_thread_should_end = 0;

};

#endif
