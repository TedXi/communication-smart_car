#include "function.h"


Pack_Processor::Pack_Processor(int kind_of_protocol,char *dev_path_or_addr){
    switch (kind_of_protocol) {
    case COMM_SERIAL:
        serial_port = new Serial_Port(dev_path_or_addr,115200);
        break;
    case COMM_NET_C:
        network_client = new NetWork_Client(dev_path_or_addr,8000,LOCAL_ID);
        if(network_client->socket_connect() == -1){
            printf("\nsocket_connect fail\n");
        }
        break;
    case COMM_NET_S:
        network_server = new NetWork_Server(8000,5,LOCAL_ID);
        break;
    default:
        printf("Pack_Processor init defeat");
        break;
    }
    communication_protocol = kind_of_protocol;
    pack_processor_thread = new thread(&Pack_Processor::PProcessor_Thread,this);
    unpack_processor_thread = new thread(&Pack_Processor::UNPProcessor_Thread,this);
}

Pack_Processor::Pack_Processor(int kind_of_protocol,char *dev_path_or_addr,int buad_rate_or_port){
    switch (kind_of_protocol) {
    case COMM_SERIAL:
        serial_port = new Serial_Port(dev_path_or_addr,buad_rate_or_port);
        break;
    case COMM_NET_C:
        network_client = new NetWork_Client(dev_path_or_addr,buad_rate_or_port,LOCAL_ID);
        network_client->socket_connect();
        break;
    case COMM_NET_S:
        network_server = new NetWork_Server(buad_rate_or_port,5,LOCAL_ID);
        break;
    default:
        printf("Pack_Processor init defeat");
        break;
    }
    communication_protocol = kind_of_protocol;
    pack_processor_thread = new thread(&Pack_Processor::PProcessor_Thread,this);
    unpack_processor_thread = new thread(&Pack_Processor::UNPProcessor_Thread,this);
}

Pack_Processor::~Pack_Processor(){
    while(send_buff.size() != 0){
        usleep(20000);
    }
    processor_thread_should_end = THREAD_END;
    if(pack_processor_thread->joinable()){pack_processor_thread->join();}
    if(communication_protocol == COMM_SERIAL || communication_protocol == COMM_NET_C)
        Send_Quit_Pack_C_or_Serial();
    if(unpack_processor_thread->joinable()){unpack_processor_thread->join();}
    delete(pack_processor_thread);
    delete(unpack_processor_thread);
    if(communication_protocol == COMM_SERIAL)
        delete(serial_port);
    else if(communication_protocol == COMM_NET_C)
        delete(network_client);
    else if(communication_protocol == COMM_NET_S)
        delete(network_server);
}

int Pack_Processor::send_data(pack_buff data){
    send_buff.push_back(data);
    g_lock.lock();
    sleep_count++;//唤醒发送线程
    g_lock.unlock();
}

void Pack_Processor::PProcessor_Thread(){
    while (processor_thread_should_end == THREAD_RUN) {
        while(sleep_count > 0){
            int tatol_length = send_buff.front().length;
            if(communication_protocol == COMM_SERIAL){
                serial_port->send_data(&tatol_length,sizeof(int));
                serial_port->send_data(&send_buff.front().sour_ID,1);
                serial_port->send_data(&send_buff.front().dest_ID,1);
                serial_port->send_data(&send_buff.front().type,1);
                serial_port->send_data(send_buff.front().buff,send_buff.front().length);
                send_buff.pop_front();
            }
            else if(communication_protocol == COMM_NET_C){
                network_client->send_data(&tatol_length,sizeof(int));
                network_client->send_data(&send_buff.front().sour_ID,1);
                network_client->send_data(&send_buff.front().dest_ID,1);
                network_client->send_data(&send_buff.front().type,1);
                network_client->send_data(send_buff.front().buff,send_buff.front().length);
                send_buff.pop_front();
            }
            else if(communication_protocol == COMM_NET_S){
                char destid = send_buff.front().dest_ID;
                network_server->send_data(&tatol_length,sizeof(int),destid);
                network_server->send_data(&send_buff.front().sour_ID,1,destid);
                network_server->send_data(&send_buff.front().dest_ID,1,destid);
                network_server->send_data(&send_buff.front().type,1,destid);
                network_server->send_data(send_buff.front().buff,send_buff.front().length,destid);
                send_buff.pop_front();
            }
            g_lock.lock();
            sleep_count--;
            g_lock.unlock();
        }
        usleep(417);//20 times per picture, 1/120/20
    }
    printf("PProcessor_Thread End!\n");
}

void Pack_Processor::UNPProcessor_Thread(){
    char *head_lenght_buff = (char *)malloc(sizeof(int));
    pack_buff data;
    while (processor_thread_should_end == THREAD_RUN) {
        if(communication_protocol == COMM_SERIAL){
            serial_port->recv_data_block(head_lenght_buff,4);
            data.length = Length_4chars2int(head_lenght_buff);
            serial_port->recv_data_block(&data.sour_ID,1);
            serial_port->recv_data_block(&data.dest_ID,1);
            serial_port->recv_data_block(&data.type,1);
            data.buff = (char *)malloc(data.length);                          //使用者必须free此内存!!
            serial_port->recv_data_block(data.buff,data.length);
            if(data.type == SEND_QUIT){
                free(data.buff);
                Send_Quit_Pack_C_or_Serial();
                processor_thread_should_end = THREAD_END;
            }
            else{
                Packet_Splitter(ref(data));
            }
        }
        else if(communication_protocol == COMM_NET_C){
            network_client->recv_data_block(head_lenght_buff,4);
            data.length = Length_4chars2int(head_lenght_buff);
            network_client->recv_data_block(&data.sour_ID,1);
            network_client->recv_data_block(&data.dest_ID,1);
            network_client->recv_data_block((char *)&data.type,1);
            data.buff = (char *)malloc(data.length);                          //使用者必须free此内存!!
            network_client->recv_data_block(data.buff,data.length);
            if(data.type == SEND_QUIT){
                free(data.buff);
                Send_Quit_Pack_C_or_Serial();
                processor_thread_should_end = THREAD_END;
            }
            else{
                Packet_Splitter(ref(data));
            }
        }
        else if(communication_protocol == COMM_NET_S){
            if(network_server->connect_fd.size() != 0){
                vector<Connect_fd> tmpbuff;
                network_server->witch_connect_readable(ref(tmpbuff));
                network_server->recv_data_block(head_lenght_buff,4,tmpbuff.front().sour_ID);
                data.length = Length_4chars2int(head_lenght_buff);
                network_server->recv_data_block(&data.sour_ID,1,tmpbuff.front().sour_ID);
                network_server->recv_data_block(&data.dest_ID,1,tmpbuff.front().sour_ID);
                network_server->recv_data_block((char *)&data.type,1,tmpbuff.front().sour_ID);
                data.buff = (char *)malloc(data.length);                          //使用者必须free此内存!!
                network_server->recv_data_block(data.buff,data.length,tmpbuff.front().sour_ID);
                if(data.type == SEND_QUIT){
                    free(data.buff);
                    Send_Quit_Pack_S(data.sour_ID);
                    int n = network_server->connect_fd.size();
                    for(int i = 0;i<n;i++){
                        if(network_server->connect_fd.at(i).sour_ID == data.sour_ID){
                            if(network_server->connect_fd.size() == 1){
                                g_lock.lock();
                                network_server->connect_fd.pop_back();
                                g_lock.unlock();
                                //processor_thread_should_end = THREAD_END;
                            }
                            else{
                                g_lock.lock();
                                for(;i<n-1;i++){
                                    network_server->connect_fd.at(i) = network_server->connect_fd.at(i+1);
                                }
                                network_server->connect_fd.pop_back();
                                g_lock.unlock();
                            }
                            break;
                        }
                    }
                }
                else{
                    Packet_Splitter(ref(data));
                }
            }
            else{
                usleep(20000);
            }
        }
    }
    free(head_lenght_buff);
    printf("UNPProcessor_Thread End!\n");
}

void Pack_Processor::get_pack(char kind_of_pack,pack_buff &ret){
    //pack_buff ret;
    ret.length = 0;
    ret.buff = NULL;
    switch (kind_of_pack) {
    case SEND_SPEED:
        if(recv_buff_SEND_SPEED.size() != 0){
            ret = recv_buff_SEND_SPEED.front();
            recv_buff_SEND_SPEED.pop_front();
        }
        break;
    case SEND_TURN_ANGLE:
        if(recv_buff_SEND_TURN_ANGLE.size() != 0){
            ret = recv_buff_SEND_TURN_ANGLE.front();
            recv_buff_SEND_TURN_ANGLE.pop_front();
        }
        break;
    case SEND_USER_OPERATION:
        //printf("size is: %d",recv_buff_SEND_SPEED.size());
        if(recv_buff_SEND_USER_OPERATION.size() != 0){
            ret = recv_buff_SEND_USER_OPERATION.front();
            recv_buff_SEND_USER_OPERATION.pop_front();
        }
        break;
    case SEND_WARNING:
        if(recv_buff_SEND_WARNING.size() != 0){
            ret = recv_buff_SEND_WARNING.front();
            recv_buff_SEND_WARNING.pop_front();
        }
        break;
    case ASK_SPEED:
        if(recv_buff_ASK_SPEED.size() != 0){
            ret = recv_buff_ASK_SPEED.front();
            recv_buff_ASK_SPEED.pop_front();
        }
        break;
    case ASK_TURN_ANGLE:
        if(recv_buff_ASK_TURN_ANGLE.size() != 0){
            ret = recv_buff_ASK_TURN_ANGLE.front();
            recv_buff_ASK_TURN_ANGLE.pop_front();
        }
        break;
    case ASK_USER_OPERATION:
        if(recv_buff_ASK_USER_OPERATION.size() != 0){
            ret = recv_buff_ASK_USER_OPERATION.front();
            recv_buff_ASK_USER_OPERATION.pop_front();
        }
        break;
    case ASK_WARNING:
        if(recv_buff_ASK_WARNING.size() != 0){
            ret = recv_buff_ASK_WARNING.front();
            recv_buff_ASK_WARNING.pop_front();
        }
        break;
    default:
        break;
    }
    //必须手动释放ret.buff
}

void Pack_Processor::Packet_Splitter(pack_buff &buff){
    switch (buff.type) {
    case SEND_SPEED:
        recv_buff_SEND_SPEED.push_back(buff);
        break;
    case SEND_TURN_ANGLE:
        recv_buff_SEND_TURN_ANGLE.push_back(buff);
        break;
    case SEND_USER_OPERATION:
        recv_buff_SEND_USER_OPERATION.push_back(buff);
        break;
    case SEND_WARNING:
        recv_buff_SEND_WARNING.push_back(buff);
        break;
    case ASK_SPEED:
        recv_buff_ASK_SPEED.push_back(buff);
        break;
    case ASK_TURN_ANGLE:
        recv_buff_ASK_TURN_ANGLE.push_back(buff);
        break;
    case ASK_USER_OPERATION:
        recv_buff_ASK_USER_OPERATION.push_back(buff);
        break;
    case ASK_WARNING:
        recv_buff_ASK_WARNING.push_back(buff);
        break;
    default:
        break;
    }
}

void Pack_Processor::Send_Quit_Pack_C_or_Serial(){
    pack_buff quit_pack;
    if(communication_protocol == COMM_SERIAL){
        quit_pack.buff = (char *)"Q";
        quit_pack.length = 2;
        quit_pack.sour_ID = LOCAL_ID;
        quit_pack.dest_ID = 0;
        quit_pack.type = SEND_QUIT;
        serial_port->send_data(&quit_pack.length,sizeof(int));
        serial_port->send_data(&quit_pack.sour_ID,1);
        serial_port->send_data(&quit_pack.dest_ID,1);
        serial_port->send_data(&quit_pack.type,1);
        serial_port->send_data(quit_pack.buff,quit_pack.length);
    }
    else if(communication_protocol == COMM_NET_C){
        quit_pack.buff = (char *)"Q";
        quit_pack.length = 2;
        quit_pack.sour_ID = network_client->local_id;
        quit_pack.dest_ID = network_client->server_id;
        quit_pack.type = SEND_QUIT;
        network_client->send_data(&quit_pack.length,sizeof(int));
        network_client->send_data(&quit_pack.sour_ID,1);
        network_client->send_data(&quit_pack.dest_ID,1);
        network_client->send_data(&quit_pack.type,1);
        network_client->send_data(quit_pack.buff,quit_pack.length);
    }
}

void Pack_Processor::Send_Quit_Pack_S(char destid){
    pack_buff quit_pack;
    quit_pack.buff = (char *)"Q";
    quit_pack.length = 2;
    quit_pack.sour_ID = network_server->local_id;
    quit_pack.dest_ID = destid;
    quit_pack.type = SEND_QUIT;
    network_server->send_data(&quit_pack.length,sizeof(int),destid);
    network_server->send_data(&quit_pack.sour_ID,1,destid);
    network_server->send_data(&quit_pack.dest_ID,1,destid);
    network_server->send_data(&quit_pack.type,1,destid);
    network_server->send_data(quit_pack.buff,quit_pack.length,destid);
}

inline int Pack_Processor::Length_4chars2int(char *length_char){
    int ret = 0;
    ret += *length_char;
    ret<<8;
    ret += *(length_char+1);
    ret<<8;
    ret += *(length_char+2);
    ret<<8;
    ret += *(length_char+3);
    return ret;
}
/******INTERNAL FUNCTION*******/
