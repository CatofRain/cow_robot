#ifndef __TTY_UART_RW_H__
#define __TTY_UART_RW_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>  
#include <errno.h>   
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <signal.h>
#include <getopt.h>
#include <linux/serial.h>
#define termios asmtermios
#include <asm/termios.h>
#undef  termios
#include <termios.h>
#include <iostream>


#define N_read 49
#define N_read_out  43

//用来记录每次的流量
//extern int prenum;
//extern int curnum;
extern int prenum[4];
extern int curnum[4];
extern int equalcnt[4] ;
extern int equalnum[4] ;
/*
int speed = 4800;
 int hardflow = 0;
 int verbose = 1;
 int flag_setZero=0;
 int fd[8];
*/
///*
//extern static const char *device;
extern  int speed ;
extern  int hardflow;
extern  int verbose ;
extern  int flag_setZero;
extern  int fdx[8];
extern int flag_setZeroR;
//*/




	
	 int libtty_setcustombaudrate(int fd, int baudrate);
	 int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity, char hardflow);
	 int libtty_open(const char *devname);
	 int libtty_close(int fd, int idx);
	 int libtty_write(int fd);
//	int libtty_write_setZero(int fd);
	 void sig_handler(int signo);
	 int libtty_read(int fd,int id);
#endif




