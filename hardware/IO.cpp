#include "IO.h"
/***********************************************************************
 * class name: Serial_Port
 **********************************************************************/
Serial_Port::Serial_Port(char *port_name,int nSpeed){
	serial_port_name = (char *)calloc(1,strlen(port_name)+1);
	strcpy(serial_port_name,port_name);
	speed = nSpeed;
	bits = 8;
	event = 'n';
	stop = 1;
	//fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
	serial_fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (-1 == serial_fd)  
    {   
		perror("Can't Open Serial Port");
    }
    if (-1 == set_opt(serial_fd,speed,bits,event,stop))  
    {   
		perror("Can't Open Serial Port");
    }
    FD_ZERO(&fdset);
    FD_SET(serial_fd,&fdset);
}

Serial_Port::Serial_Port(char *port_name,int nSpeed,int nBits,char nEvent,int nStop){
	serial_port_name = (char *)calloc(1,strlen(port_name)+1);
	strcpy(serial_port_name,port_name);
	speed = nSpeed;
	bits = nBits;
	event = nEvent;
	stop = nStop;
	//fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);				//阻塞读写
	serial_fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);		//非阻塞读写
    if (-1 == serial_fd)  
    {
		perror("Can't Open Serial Port"); 
    }
    if (-1 == set_opt(serial_fd,speed,bits,event,stop))  
    {   
		perror("Can't Open Serial Port");
    }
    FD_ZERO(&fdset);
    FD_SET(serial_fd,&fdset);
}

Serial_Port::~Serial_Port(){
	free(serial_port_name);
    close(serial_fd);
}

int Serial_Port::recv_data(void *buff, unsigned int char_num){
	return read(serial_fd,buff,char_num);//读串口   
}

int Serial_Port::recv_data_block(void *buff, unsigned int char_num){
    for(int i = 0;i<char_num;){
        select(serial_fd+1, &fdset, NULL, NULL, NULL);
        i += read(serial_fd,(char *)buff+i,char_num-i);//读串口
    }
    return 0;
}

int Serial_Port::recv_data_block(void *buff, unsigned int char_num,unsigned int timeout)
{
    timeval time;
    time.tv_sec = timeout;
    time.tv_usec = 0;
    int rec = select(serial_fd+1, &fdset, NULL, NULL, &time);
    if(rec == 0){
        return -1;
    }
    int i = 0;
    do{
        i += read(serial_fd,(char *)buff+i,char_num-i);//读串口
    }
    while(i<char_num && select(serial_fd+1, &fdset, NULL, NULL, &time));
    return 0;
}

int Serial_Port::send_data(void *buff, unsigned int lenth){
    return write(serial_fd,buff,lenth);//写串口
}

int Serial_Port::set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{  
/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
     if( tcgetattr( fd,&oldtio)  !=  0) {    
		perror("SetupSerial 1");  
		printf("tcgetattr( fd,&oldtio) -> %d\n",tcgetattr( fd,&oldtio));   
		return -1;   
     }  
     bzero( &newtio, sizeof( newtio ) );   
/*步骤一，设置字符大小*/   
     newtio.c_cflag  |=  CLOCAL | CREAD;    
     newtio.c_cflag &= ~CSIZE;    
/*设置停止位*/   
     switch( nBits )   
     {   
     case 7:   
      newtio.c_cflag |= CS7;   
      break;   
     case 8:   
      newtio.c_cflag |= CS8;   
      break;   
     }   
/*设置奇偶校验位*/   
     switch( nEvent )   
     {   
     case 'o':  
     case 'O': //奇数   
      newtio.c_cflag |= PARENB;   
      newtio.c_cflag |= PARODD;   
      newtio.c_iflag |= (INPCK | ISTRIP);   
      break;   
     case 'e':  
     case 'E': //偶数   
      newtio.c_iflag |= (INPCK | ISTRIP);   
      newtio.c_cflag |= PARENB;   
      newtio.c_cflag &= ~PARODD;   
      break;  
     case 'n':  
     case 'N':  //无奇偶校验位   
      newtio.c_cflag &= ~PARENB;   
      break;  
     default:  
      break;  
     }   
     /*设置波特率*/   
switch( nSpeed )   
     {   
     case 2400:   
      cfsetispeed(&newtio, B2400);   
      cfsetospeed(&newtio, B2400);   
      break;   
     case 4800:   
      cfsetispeed(&newtio, B4800);   
      cfsetospeed(&newtio, B4800);   
      break;   
     case 9600:   
      cfsetispeed(&newtio, B9600);   
      cfsetospeed(&newtio, B9600);   
      break;   
     case 115200:   
      cfsetispeed(&newtio, B115200);   
      cfsetospeed(&newtio, B115200);   
      break;   
     case 460800:   
      cfsetispeed(&newtio, B460800);   
      cfsetospeed(&newtio, B460800);   
      break;   
     default:   
      cfsetispeed(&newtio, B9600);   
      cfsetospeed(&newtio, B9600);   
     break;   
     }   
/*设置停止位*/   
     if( nStop == 1 )   
      newtio.c_cflag &=  ~CSTOPB;   
     else if ( nStop == 2 )   
      newtio.c_cflag |=  CSTOPB;   
/*设置等待时间和最小接收字符*/   
     newtio.c_cc[VTIME]  = 0;   
     newtio.c_cc[VMIN] = 0;   
/*处理未接收字符*/   
     tcflush(fd,TCIFLUSH);   
/*激活新配置*/   
if((tcsetattr(fd,TCSANOW,&newtio))!=0)   
     {   
      perror("com set error");   
      return -1;   
     }   
     //printf("Serial port set done!\n");   
     return 0;   
}
