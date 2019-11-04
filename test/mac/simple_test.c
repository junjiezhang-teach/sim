//
// Created by Xiaozhong Guo on 11/2/19.
//
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libserialport.h"
#include <unistd.h>

const char *desired_port = "COM1";
struct sp_port *port;
enum sp_return;

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

void parse_serial(char *byte_buff, int byte_num) {
    for (int i = 0; i < byte_num; i++) {
        printf("%c", byte_buff[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    printf("start");
    list_ports();

    printf("Opening port '%s' \n", desired_port);
    enum sp_return error = sp_get_port_by_name(desired_port, &port);
    if (error == SP_OK) {
        error = sp_open(port, SP_MODE_READ);
        if (error == SP_OK) {
            sp_set_baudrate(port, 57600);
            while (1) {

                sleep(200); // can do something else in mean time
                int bytes_waiting = sp_input_waiting(port);
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

