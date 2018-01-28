#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include "../public.h"
#include "../hardware/IO.h"
#include "../hardware/socket.h"

#define LOCAL_ID 1

#define COMM_SERIAL 1
#define COMM_NET_C 2
#define COMM_NET_S 3
#define COMM_UNDEFINED 4


#define THREAD_RUN 1
#define THREAD_END 2

//#define HEAD_LENGTH (sizeof(pack_buff.length)+sizeof(pack_buff.ID)+sizeof(pack_buff.type))//6

class Pack_Processor
{
public:
    Pack_Processor(int kind_of_protocol, char *dev_path_or_addr);
    Pack_Processor(int kind_of_protocol, char *dev_path_or_addr, int buad_rate_or_port);
    ~Pack_Processor();
    int send_data(pack_buff data);
    void get_pack(char kind_of_pack,pack_buff &ret);

private:
    void PProcessor_Thread();
    void UNPProcessor_Thread();
    inline int Length_4chars2int(char *length_char);
    void Packet_Splitter(pack_buff &buff);
    void Send_Quit_Pack_C_or_Serial();
    void Send_Quit_Pack_S(char destid);
    thread *pack_processor_thread;
    thread *unpack_processor_thread;
    Serial_Port *serial_port = NULL;
    NetWork_Client *network_client = NULL;
    NetWork_Server *network_server = NULL;
    mutex g_lock;

    deque<pack_buff> recv_buff_SEND_TURN_ANGLE;
    deque<pack_buff> recv_buff_SEND_SPEED;
    deque<pack_buff> recv_buff_SEND_WARNING;
    deque<pack_buff> recv_buff_SEND_USER_OPERATION;
    deque<pack_buff> recv_buff_ASK_TURN_ANGLE;
    deque<pack_buff> recv_buff_ASK_SPEED;
    deque<pack_buff> recv_buff_ASK_WARNING;
    deque<pack_buff> recv_buff_ASK_USER_OPERATION;
    deque<pack_buff> send_buff;//用于暂存代待发送数据
    int communication_protocol = COMM_UNDEFINED;
    int processor_thread_should_end = THREAD_RUN;
    int sleep_count = 0;
};

#endif
