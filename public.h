/*************************************************
 * 全程序遵循谁用谁free/delete的原则
 *
 *************************************************/
#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <vector>
#include <deque>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <math.h>
//#include <arm_neon.h>

using namespace std;

#define ERR -1

enum packet_enum:char
{
    SEND_QUIT = 0xff,
    SEND_TURN_ANGLE = 0,
    SEND_SPEED,//the speed of each wheel
    SEND_WARNING,//Buzzer
    SEND_USER_OPERATION,

    ASK_TURN_ANGLE = 80,//并非0x80!
    ASK_SPEED,//the speed of each wheel
    ASK_WARNING,//Buzzer
    ASK_USER_OPERATION,
};


#pragma pack(push) //保存对齐状态
#pragma pack(4)// 设定为4字节对齐
struct pack_buff
{
    char *buff;
    int length;
    char sour_ID;
    char dest_ID;
    packet_enum type;
};
#pragma pack(pop)// 恢复对齐状态

struct data_buff
{
    char *buff;
    int length;
};

#endif
