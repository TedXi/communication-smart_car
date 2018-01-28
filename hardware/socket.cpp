#include "socket.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

NetWork_Client::NetWork_Client(char* addr,unsigned int port,char local_id_in){
	flage_connected = 0;
	server_addr = (char *)calloc(1,strlen(addr)+1);
	if(addr != NULL){
		strcpy(server_addr,addr);
	}
	else{
		printf("Net Server IP is NULL");
	}
	server_addr_int32 = ip_addr2int32(server_addr);
	server_port = port;
    local_id = local_id_in;
	bzero(&addr_ser,sizeof(addr_ser));
	addr_ser.sin_family=AF_INET;
	addr_ser.sin_addr.s_addr=htonl(server_addr_int32);
	addr_ser.sin_port=htons(server_port);

    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    socket_fd = sockfd;
    if(sockfd==-1)
    {

    }

    FD_ZERO(&fdset);
}

NetWork_Client::~NetWork_Client(){
	free(server_addr);
    close(socket_fd);
}

unsigned int NetWork_Client::get_server_int32_addr(){
	return server_addr_int32;
}

unsigned int NetWork_Client::get_server_port(){
	return server_port;
}

unsigned int NetWork_Client::socket_connect(){
    int err=connect(socket_fd,(struct sockaddr *)&addr_ser,sizeof(addr_ser));
	if(err==-1)
	{
		return -1;
	}
    write(socket_fd,&local_id,1);
    usleep(NETWORK_DELAY_TIME);//25ms
    if(read(socket_fd,&server_id,1)){
        printf("server_id %d\n",server_id);
        flage_connected = 1;
    }
    else{
        printf("get server ID time out\n");
        close(socket_fd);
    }
	return 1;
}

int NetWork_Client::send_data(void *data,unsigned int lenth){
	return send(socket_fd,data,lenth,0);
}

int NetWork_Client::recv_data(char *buff, int char_num){				//char_num is how many chars would you like to read
	return recv(socket_fd,buff,char_num,0);
}

int NetWork_Client::recv_data_block(char *buff, int char_num){
    FD_ZERO(&fdset);
    FD_SET(socket_fd,&fdset);
    for(int i = 0;i<char_num;){
        select(socket_fd+1, &fdset, NULL, NULL, NULL);
        i += read(socket_fd,buff+i,char_num-i);//读串口
    }
    return 0;
}

int NetWork_Client::recv_data_block(char *buff, int char_num, unsigned int timeout){
    timeval time;
    time.tv_sec = timeout;
    time.tv_usec = 0;
    FD_ZERO(&fdset);
    FD_SET(socket_fd,&fdset);
    int rec = select(socket_fd+1, &fdset, NULL, NULL, &time);
    if(rec == 0){
        return -1;
    }
    int i = 0;
    do{
        i += read(socket_fd,buff+i,char_num-i);//读串口
    }
    while(i<char_num && select(socket_fd+1, &fdset, NULL, NULL, &time));
    return 0;
}

unsigned int NetWork_Client::ip_addr2int32(char* addr_input){
	int i,i1;
	unsigned int addr_int32 = 0;
	char *addr = (char *)calloc(1,strlen(addr_input)+1);
	strcpy(addr,addr_input);
	for(i1 = 0;i1<4;i1++){
		for(i = 0;*addr != '.' && *addr != 0;i++){
			addr++;
		}
		switch(i){
		case 1:
			addr_int32 += *(addr-1)-48;
		break;
		case 2:
			addr_int32 += (*(addr-2)-48)*10+*(addr-1)-48;
		break;
		case 3:
			addr_int32 += (*(addr-3)-48)*100+ (*(addr-2)-48)*10+*(addr-1)-48;
		break;
		}
		if(i1<3)
			addr_int32 = addr_int32<<8;
		addr++;
	}
	return addr_int32;
}

NetWork_Server::NetWork_Server( int port, int max_listen_num, char local_id_in){
        listen_port = port;
        max_client = max_listen_num;
        local_id = local_id_in;
        server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&myaddr, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_port = htons(port);
        if(bind(server_sock_fd,(struct sockaddr*)(&myaddr), sizeof(struct sockaddr)) < 0){
            fprintf(stderr, "NetWork_Server bind failed \n");
        }
        if(listen(server_sock_fd, max_client) != 0){
            fprintf(stderr, "NetWork_Server listen failed \n");
        }
        Accept_Thread = new thread(&NetWork_Server::get_new_connection,this);
        accept_thread_should_end = 0;
}

NetWork_Server::~NetWork_Server(){
    accept_thread_should_end = 1;
    close(server_sock_fd);
    if(Accept_Thread->joinable()){
        Accept_Thread->join();
    }
    while(connect_fd.size()){
        close(connect_fd.back().fd);
        connect_fd.pop_back();
    }
}

void NetWork_Server::get_new_connection(){
    int  sin_size = sizeof(struct sockaddr_in);
    Connect_fd newsockfd;
    fd_set accept_fdset;
    FD_ZERO(&accept_fdset);
    FD_SET(server_sock_fd,&accept_fdset);
    struct timeval timeout;//1s
    while(accept_thread_should_end == 0){
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        int nfds = select(server_sock_fd+1,&accept_fdset,NULL,NULL,&timeout);
        //printf("select time out or get info nfds: %d\n",nfds);
        if(nfds>0){
            newsockfd.fd = accept(server_sock_fd, NULL, (socklen_t *)&sin_size);
            if (newsockfd.fd > 0){
                usleep(NETWORK_DELAY_TIME);//25ms
                if(read(newsockfd.fd,&newsockfd.sour_ID,1)){
                    printf("get client ID at %d\n",newsockfd.sour_ID);
                    write(newsockfd.fd,&local_id,1);
                    g_lock.lock();
                    connect_fd.push_back(newsockfd);
                    g_lock.unlock();
                }
                else{
                    printf("get client ID time out\n");
                    close(newsockfd.fd);
                }
            }
        }
        else{
            FD_ZERO(&accept_fdset);
            FD_SET(server_sock_fd,&accept_fdset);
        }
    }
    printf("get_new_connection thread end\n");
}

int NetWork_Server::send_data(void *data, unsigned int lenth,char send_id){
    for(int i = 0; i<connect_fd.size();i++){
        if(connect_fd.at(i).sour_ID == send_id){
            write(connect_fd.at(i).fd,data,lenth);
            return 1;
        }
    }
    printf("Thread is no send_id at vector connect_fd\n");
    return 0;
}

int NetWork_Server::recv_data_block(char *buff, int char_num,char recv_id){
    fd_set tmpfdset;
    FD_ZERO(&tmpfdset);
    for(int i = 0; i<connect_fd.size();i++){
        if(connect_fd.at(i).sour_ID == recv_id){
            int tmp_fd = connect_fd.at(i).fd;
            FD_SET(tmp_fd,&tmpfdset);
            for(int i1 = 0; i1<char_num;){
                if(select(tmp_fd+1, &tmpfdset, NULL, NULL, NULL) && !FD_ISSET(tmp_fd,&tmpfdset)){
                    printf("recv_data_block failed, select is true but FD_ISSET is false\n");
                }
                i1 += read(tmp_fd,buff+i1,char_num-i1);
            }
            return 1;
        }
    }
    printf("Thread is no send_id at vector connect_fd\n");
    return 0;
}

void NetWork_Server::witch_connect_readable(vector<Connect_fd> &fdin){
    fd_set fdset;
    int max_fd = 0;
    timeval renewfd;
    int n;
    //vector<Connect_fd> fdin;
    printf("in witch_connect_readable\n");
    loop:
    FD_ZERO(&fdset);
    renewfd.tv_sec = 0;
    renewfd.tv_usec = 100000;
    n = connect_fd.size();
    for(int i = 0;i<n;i++){
        int tmp_fd = connect_fd.at(i).fd;
        FD_SET(tmp_fd,&fdset);
        if(tmp_fd > max_fd){
            max_fd = tmp_fd;
        }
    }
    if(select(max_fd+1, &fdset, NULL, NULL, &renewfd)){
        for(int i = 0;i<n;i++){
            if(FD_ISSET(connect_fd.at(i).fd,&fdset)){
                fdin.push_back(connect_fd.at(i));
                printf("connect_fd push_backed\n");
            }
        }
    }
    else{
        goto loop;
    }
    printf("fdin.at(0).fd is %d\n",fdin.at(0).fd);
    //return fdin;
    printf("out witch_connect_readable\n");
}

void NetWork_Server::get_connects(vector<Connect_fd> &fdin){
    fdin.clear();
    fdin.insert(fdin.end(),connect_fd.begin(),connect_fd.end());
}

unsigned int NetWork_Server::ip_addr2int32(char* addr_input){
    int i,i1;
    unsigned int addr_int32 = 0;
    char *addr = (char *)calloc(1,strlen(addr_input)+1);
    strcpy(addr,addr_input);
    for(i1 = 0;i1<4;i1++){
        for(i = 0;*addr != '.' && *addr != 0;i++){
            addr++;
        }
        switch(i){
        case 1:
            addr_int32 += *(addr-1)-48;
        break;
        case 2:
            addr_int32 += (*(addr-2)-48)*10+*(addr-1)-48;
        break;
        case 3:
            addr_int32 += (*(addr-3)-48)*100+ (*(addr-2)-48)*10+*(addr-1)-48;
        break;
        }
        if(i1<3)
            addr_int32 = addr_int32<<8;
        addr++;
    }
    return addr_int32;
}
