#ifndef _IO_H_
#define _IO_H_

#include "../public.h"
#include <unistd.h>  
#include <fcntl.h>
#include <termios.h>

class Serial_Port
{
public:
	/*******************************************
	 * char *port_name: Serial Port path
	 * int nSpeed: Baud of Serial, like 9600 or 115200
	 * int nBits: Bit of once, 7 or 8
	 * char nEvent: Check Bit, o(O),e(E) or n(N)
	 * int nStop: Stop Bit, 1 or 2
	 *******************************************/
	Serial_Port(char *port_name,int nSpeed);
	Serial_Port(char *port_name,int nSpeed,int nBits,char nEvent,int nStop);//串口名称,波特率,位数,校验,停止位
	~Serial_Port();
	/*******************************************
	 * 1.void *buff: the buffer of send data
	 * 2.unsigned int lenth: the lenth of send data
     * 3.return: the num of data real write
	 ******************************************/
	int send_data(void *buff, unsigned int lenth);
	/*******************************************
	 * 1.void *buff: the buffer of recv data
	 * 2.unsigned int char_num: the max data can be recv
	 * 3.return: the num of data real recved
	 ******************************************/
	int recv_data(void *buff, unsigned int char_num);
    int recv_data_block(void *buff, unsigned int char_num);
    int recv_data_block(void *buff, unsigned int char_num,unsigned int timeout);
private:
	char *serial_port_name;
	char event;
	int speed,bits,stop;
	int serial_fd;
    fd_set fdset;
	struct termios newtio,oldtio;
	
	int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
};
#endif
