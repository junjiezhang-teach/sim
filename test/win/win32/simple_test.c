//
// Created by guoxi on 2019/11/1.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libserialport.h"
#include <windows.h>


#define COMFARME_WAIETIME 2000
const char *desired_port = "COM1";
struct sp_port *port;
enum sp_return;

enum sp_return  sp_ret = SP_OK;


void list_ports() {
    int i;
    struct sp_port **ports;

    enum sp_return error = sp_list_ports(&ports);
    if (error == SP_OK) {
        for (i = 0; ports[i]; i++) {
            printf("Found port: '%s'\n", sp_get_port_name(ports[i]));
        }
        sp_free_port_list(ports);
    } else {
        printf("No serial devices detected\n");
    }
    printf("\n");
}
/*
void parse_serial(char *byte_buff, int byte_num) {
    printf("%d", byte_num);
    for (int i = 0; i < byte_num; i++) {
        printf("%c", byte_buff[i]);

    }
    printf("\n");
}*/
static void PrintfData(unsigned char *data, int len)
{
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
}
int SlipSend(unsigned short int DataLength, unsigned char * lpdata, unsigned char * Sbuf)
{
    int i,j=0;
    Sbuf[j++] = 0xC0;
    for(i = 0;i < DataLength;i++)
    {
        switch(lpdata[i])
        {
            case 0xC0:
                Sbuf[j++] = 0xDB;Sbuf[j++] = 0xDC;break;
            case 0xDB:
                Sbuf[j++] = 0xDB;Sbuf[j++] = 0xDD;break;
            default:
                Sbuf[j++] = lpdata[i];break;
        }
    }
    Sbuf[j++] = 0xC0;
    Sbuf[j] = '\0';
    return j;
}
int SlipRead(unsigned short int DataLength, unsigned char * lpdata, unsigned char * Rbuf)
{
    int i = 0,j = 0;
    do
    {
        switch(Rbuf[i])
        {
            case 0xC0:break;
            case 0xDB:
            {
                i += 1;
                switch(Rbuf[i])
                {
                    case 0xDC:
                        lpdata[j++] = 0xC0;break;
                    case 0xDD:
                        lpdata[j++] = 0xDB;break;
                }
                break;
            }
            default:
                lpdata[j++] = Rbuf[i];break;
        }
        i+=1;
    }while (i < DataLength);
    lpdata[j] = '\0';
    return j;
}
//数据切换（上位机->采集电路）
void change_s(struct sp_port *port)
{
    char change_buff[4] ={0xC0,0xCF,0xCF,0xC0};
    sp_ret = sp_blocking_write(port, change_buff, 4,COMFARME_WAIETIME);
    //sp_drain(port);
    if (sp_ret <= 0) {
        printf("sp_nonblocking_write error @ret %d @errmsg %s\n", sp_ret, sp_last_error_message());
    }/*
    else {
        PrintfData(change_buff, sp_ret);
    }*/
}
//数据停止发送指令（上位机->采集电路）
void end_s(struct sp_port *port)
{
    char end_buff[4]={0xC0,0xF8,0xF8,0xC0};
    sp_ret = sp_blocking_write(port,end_buff,4,COMFARME_WAIETIME);
    if (sp_ret <= 0)
    {
        printf("sp_nonblocking_write error @ret %d @errmsg %s\n", sp_ret, sp_last_error_message());
    }/*
    else
    {
        PrintfData(end_buff, sp_ret);
    }*/
}void begin_s(struct sp_port *port)
{
    char begin_buff[4]={0xC0,0xF7,0xF7,0xC0};
    sp_ret = sp_blocking_write(port,begin_buff,4,COMFARME_WAIETIME);
    sp_drain(port);
    if(sp_ret <= 0)
    {
        printf("sp_nonblocking_write error @ret %d @errmsg %s\n", sp_ret, sp_last_error_message());
    } /*else{
                PrintfData(begin_buff,sp_ret);
            }*/
}
void sol_par_r(struct sp_port *port)
{
    char solution_buf[5];
    //for(i = 0;i < 6;i++) {
    solution_buf[0] = 0xC0;
    solution_buf[1] = 0xA7;
    solution_buf[2] = 0;//0~5
    solution_buf[3] = solution_buf[1] ^ solution_buf[2];
    solution_buf[4] = 0xC0;
    sp_ret = sp_blocking_write(port, solution_buf, 5,COMFARME_WAIETIME);
    if (sp_ret <= 0) {
        printf("sp_nonblocking_write error @ret %d @errmsg %s\n", sp_ret, sp_last_error_message());
    }/*
    else {
        PrintfData(solution_buf, sp_ret);
    }*/
    //}
}
void parse_serial(unsigned char* byte_buff,int byte_num)
{
    int i=0;
    //int k = 0;

    float SerialData[8];//时间 6通道数据 温度
    for(i=0;i<byte_num;i++)
    {
        begin_s(port);
        if(byte_buff[i]==0xA5)
        {
            if((byte_num-i)<35) //小于一包数据不处理
            {
                return ;
            }

            if(byte_buff[i+35]!=0x0D) //包最后一字节不是0x0D
            {
                continue;
            }

            SerialData[0] = *(float* )&byte_buff[i+1];
            SerialData[1] = *(float* )&byte_buff[i+5];
            SerialData[2] = *(float* )&byte_buff[i+9];
            SerialData[3] = *(float* )&byte_buff[i+13];
            SerialData[4] = *(float* )&byte_buff[i+17];
            SerialData[5] = *(float* )&byte_buff[i+21];
            SerialData[6] = *(float* )&byte_buff[i+25];
            SerialData[7] = *(float* )&byte_buff[i+29];

            //memcpy(&SerialData[0],&byte_buff[i+1],32);
            /*
            for (k = 0; k < 8; k++) {
                SerialData[k] = *(float *)&byte_buff[i + 1];
                i += 4;
            }*/
            printf("时间:%f, 1数据：%f,2数据：%f,3数据：%f,4数据：%f,5数据：%f,6数据：%f,温度：%f\n",
                   SerialData[0],SerialData[1],SerialData[2],SerialData[3],SerialData[4],SerialData[5],
                   SerialData[6],SerialData[7]);
            enum sp_return  sp_ret = SP_OK;
            //数据切换（上位机->采集电路）
            change_s(port);
            //数据停止发送指令（上位机->采集电路）
            end_s(port);
            //解算系数读取（上位机->采集电路）
            sol_par_r(port);

            //解算系数写入（上位机->采集电路）
            //C0 DB 数据（51）
            unsigned char solution_l_buf[53];
            unsigned char solution_r_buf[57];
            //100 0000     ->  40 42 0F 00
            solution_l_buf[0] = 0xC0;
            solution_l_buf[1] = 0xDB;
            solution_l_buf[2] = 0x57;
            for(int j = 3;j < 51;j = j+4)
            {
                solution_l_buf[j] = 0x40;
                solution_l_buf[j+1] = 0x42;
                solution_l_buf[j+2] = 0x0F;
                solution_l_buf[j+3] = 0x00;
            }
            solution_l_buf[51] = 0;//0~5
            solution_l_buf[52] = solution_l_buf[2];
            for(i = 3;i < 53;i++ )
            {
                solution_l_buf[52] ^= solution_l_buf[i];
            }
            sp_ret = sp_blocking_write(port, solution_l_buf,53,COMFARME_WAIETIME);
            if(sp_ret <= 0)
            {
                printf("sp_nonblocking_write error @ret %d @errmsg %s\n",sp_ret, sp_last_error_message());
            }/*
            else{
                PrintfData(solution_l_buf,53);
            }*/
            SlipSend(53,solution_l_buf,solution_r_buf);
            //PrintfData(solution_r_buf,57);
            //结算系数显示（采集电路->上位机）
            unsigned char solution_c_buf[53];
            unsigned  char solution_cc_buf[52];
            int s_num = 0;
            SlipRead(57,solution_c_buf,solution_r_buf);
            //PrintfData(solution_c_buf,53);
            //显示的时候没有组号
            for(i = 0;i < 51;i++)
            {
                solution_cc_buf[i] = solution_c_buf[i];
            }
            solution_cc_buf[51] = solution_c_buf[52];

            s_num =sp_blocking_read(port,solution_cc_buf,52,COMFARME_WAIETIME);
            //printf("%d\n",s_num);
            float  solution_dis_data[12];
            for(i = 0;i<s_num;i++)
            {
                if (solution_cc_buf[i] == 0x57) {
                    for (int k = 0; k < 12; k++) {
                        solution_dis_data[k] = *(float *) &solution_cc_buf[i+1];
                        i += 4;
                    }
                    printf("结算系数显示    系数1：%f,系数2：%f,系数3：%f,系数4：%f,系数5：%f,系数6：%f,"
                           "系数7：%f,系数8：%f,系数9：%f,系数10：%f,系数11：%f,系数12：%f\n",
                           solution_dis_data[0], solution_dis_data[1], solution_dis_data[2], solution_dis_data[3],
                           solution_dis_data[4], solution_dis_data[5], solution_dis_data[6], solution_dis_data[7],
                           solution_dis_data[8], solution_dis_data[9], solution_dis_data[10], solution_dis_data[11]);
                }
            }

            //零值参数读取（上位机->采集电路）
            char Read_buf[4]={0xC0,0xA9,0xA9,0xC0};
            sp_ret = sp_blocking_write(port,Read_buf,4,COMFARME_WAIETIME);
            if(sp_ret <= 0)
            {
                printf("sp_nonblocking_write error @ret %d @errmsg %s\n",sp_ret, sp_last_error_message());
            } /*else{
                PrintfData(Read_buf,4);
            }*/
            //零值参数写入（上位机->采集电路）
            // C0 DB 数据（26）
            unsigned char zero_l_buf[28];
            unsigned char zero_r_buf[32];
            //100 0000     ->  40 42 0F 00
            /*
            int i1 = 1000000;
            char *p;
            p = (char *)&i1;
            for(i = 0;i < 4;i++)
            {
                printf("%02x\n",*p++);
            }
            */
            zero_l_buf[0] = 0xC0;
            zero_l_buf[1] = 0xDB;
            zero_l_buf[2] = 0x59;
            for(int j = 3;j < 27;j = j+4)
            {
                zero_l_buf[j] = 0x40;
                zero_l_buf[j+1] = 0x42;
                zero_l_buf[j+2] = 0x0F;
                zero_l_buf[j+3] = 0x00;
            }
            zero_l_buf[27] = zero_l_buf[2];
            for(i = 3;i < 27;i++ )
            {
                zero_l_buf[27] ^= zero_l_buf[i];
            }
            sp_ret = sp_blocking_write(port, zero_l_buf,28,COMFARME_WAIETIME);
            if(sp_ret <= 0)
            {
                printf("sp_nonblocking_write error @ret %d @errmsg %s\n",sp_ret, sp_last_error_message());
            }
            /*else{
                PrintfData(zero_l_buf,28);
            }*/
            SlipSend(28,zero_l_buf,zero_r_buf);
            //PrintfData(zero_r_buf,32);
            //零值参数显示（采集电路->上位机）
            unsigned char zero_c_buf[28];
            int num = 0;
            SlipRead(32,zero_c_buf,zero_r_buf);
            //PrintfData(zero_c_buf,28);
            if(zero_c_buf[2] == 0x59)
                zero_c_buf[2] = 0xB9;
            //num = sp_nonblocking_read(port,zero_c_buf,28);
            sp_blocking_read(port,zero_c_buf,28,COMFARME_WAIETIME);
            //printf("%d\n",num);
            float  zero_dis_data[6];i =2;
            //for(i = 2;i<num;i++) {
                //if (zero_c_buf[i] == 0xB9) {
                    for (int k = 0; k < 6; k++) {
                        zero_dis_data[k] = *(float *) &zero_c_buf[i+1];
                        i += 4;
                    }
                    printf("--------------\n零值参数显示\n系数1：%f,系数2：%f,系数3：%f,系数4：%f,系数5：%f,系数6：%f\n-----------------\n",
                            zero_dis_data[0], zero_dis_data[1], zero_dis_data[2], zero_dis_data[3], zero_dis_data[4], zero_dis_data[5]);
                //}
            //}
            i+=35;
            //数据开始发送指令（上位机->采集电路）
            begin_s(port);
        }

    }

}





int main(int argc, char *argv[]) {
    printf("start\n");

    list_ports();

    printf("Opening port '%s' \n", desired_port);
    enum sp_return error = sp_get_port_by_name(desired_port, &port);
    if (error == SP_OK) {
        error = sp_open(port, SP_MODE_READ_WRITE);
        printf("Port '%s' Opened\n", desired_port);
        if (error == SP_OK) {
            sp_set_baudrate(port, 921600);
            sp_set_parity(port,SP_PARITY_NONE);
            sp_set_bits(port,8);
            sp_set_stopbits(port,1);
            while (1) {
                Sleep(200); // can do something else in mean time
                int bytes_waiting = sp_input_waiting(port);//输入端口等待
                if (bytes_waiting > 0) {
                    printf("Bytes waiting %i\n", bytes_waiting);
                    char byte_buff[512];
                    int byte_num = 0;
                    byte_num = sp_nonblocking_read(port, byte_buff, 512);
                    parse_serial(byte_buff, byte_num);
                }
                fflush(stdout);
            }

            sp_close(port);
        } else {
            printf("Error opening serial device\n");
        }
    } else {
        printf("Error finding serial device\n");
    }
    return 0;
}