#include <iostream>
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
#define termios asmtermios
#include <asm/termios.h>
#undef  termios
#include <linux/serial.h>
#include <termios.h>
#include <vector>
#include <iostream>
extern "C"{
extern int ioctl(int d, int request, ...);
}
using namespace std;

int libtty_setcustombaudrate(int fd, int baudrate)
{
	struct termios2 tio;

	if (ioctl(fd, TCGETS2, &tio)) {
		perror("TCGETS2");
		return -1;
	}

	tio.c_cflag &= ~CBAUD;
	tio.c_cflag |= BOTHER;
	tio.c_ispeed = baudrate;
	tio.c_ospeed = baudrate;

	if (ioctl(fd, TCSETS2, &tio)) {
		perror("TCSETS2");
		return -1;
	}

	if (ioctl(fd, TCGETS2, &tio)) {
		perror("TCGETS2");
		return -1;
	}

	return 0;
}

int libtty_open(const char *devname)
{
	int fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY | O_TRUNC); 
	int flags = 0;
	
	if (fd < 0) {                        
		perror("open device failed");
		return -1;            
	}
	
	flags = fcntl(fd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("fcntl failed.\n");
		return -1;
	}
		
	if (isatty(fd) == 0) {
		printf("not tty device.\n");
		return -1;
	}
	else
		printf("tty device test ok.\n");
	
	return fd;
}

int libtty_setopt(int fd, int speed, int databits, int stopbits, char parity, char hardflow)
{
	struct termios newtio;
	struct termios oldtio;
	int i;

	bzero(&newtio, sizeof(newtio));
	bzero(&oldtio, sizeof(oldtio));

	if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");
		return -1;
	}
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	/* set data bits */
	switch (databits) {
	case 5:
		newtio.c_cflag |= CS5;
		break;
	case 6:
		newtio.c_cflag |= CS6;
		break;
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "unsupported data size\n");
		return -1;
	}

	/* set parity */
	switch (parity) {
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB;    /* Clear parity enable */
		newtio.c_iflag &= ~INPCK;     /* Disable input parity check */
		break;
	case 'o':
	case 'O':
		newtio.c_cflag |= (PARODD | PARENB); /* Odd parity instead of even */
		newtio.c_iflag |= INPCK;     /* Enable input parity check */
		break;
	case 'e':
	case 'E':
		newtio.c_cflag |= PARENB;    /* Enable parity */
		newtio.c_cflag &= ~PARODD;   /* Even parity instead of odd */
		newtio.c_iflag |= INPCK;     /* Enable input parity check */
		break;
	default:
		fprintf(stderr, "unsupported parity\n");
		return -1;
	}

	/* set stop bits */
	switch (stopbits) {
	case 1:
		newtio.c_cflag &= ~CSTOPB;
		break;
	case 2:
		newtio.c_cflag |= CSTOPB;
		break;
	default:
		perror("unsupported stop bits\n");
		return -1;
	}

	if (hardflow)
		newtio.c_cflag |= CRTSCTS;
	else
		newtio.c_cflag &= ~CRTSCTS;

	newtio.c_cc[VTIME] = 20;	/* Time-out value (tenths of a second) [!ICANON]. */
	newtio.c_cc[VMIN] = 1;	/* Minimum number of bytes read at once [!ICANON]. */

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) != 0) {
		perror("tcsetattr");
		return -1;
	}


	if (libtty_setcustombaudrate(fd, speed) != 0) {
		perror("setbaudrate");
		return -1;
	}

	return 0;
}

void printvec(vector<uint8_t> v){
	if(v.size()<300 || v.size()>500) return;
	int num=0;
	for(int i=0;i<v.size();i++){
		printf(" 0x%.2x",(uint8_t)v[i]);
	}
	for(int i=9;i<v.size();i=i+6){
		int32_t temp=(int32_t)(v[i]<<8|v[i+1]<<16|v[i+2]<<24)/256;
		float result=temp/1000.0f;
		num++;
		cout<<result<<" ";
	}
		cout<<num<<endl;
}
char buf[2048];
int pflag=0;
vector<uint8_t> res;

int main(int argc, char* argv[])
{
	int fd =libtty_open((char *)"/dev/ttyUSB0");
	if(fd<0){
		cout<<"libtty_read error"<<endl;
		exit;
	}
	int ret=libtty_setopt(fd,921600,8,1,'n',0);
	if(ret!=0){
		cout<<"libtty_read error"<<endl;
	}
	uint8_t wbuf[]={0x57,0x10,0xFF,0xFF,0x00,0xFF,0xFF,0x63};
	while(1){
		memset(buf,0,2048);
		write(fd,wbuf,sizeof(wbuf));
		ret = read(fd,buf,sizeof(buf));
		cout<<ret<<endl;
		for(int i=0;i<ret;i++){
			printf(" 0x%.2x",(uint8_t)buf[i]);
		}
		cout<<"--------------------"<<endl;
		for(int i=9;i<ret;i=i+6){
			int32_t temp=(int32_t)((uint8_t)buf[i]<<8|(uint8_t)buf[i+1]<<16|(uint8_t)buf[i+2]<<24)/256;
			float result=temp/1000.0f;
			cout<<result<<" ";
		}
		cout<<"--------------"<<endl;
		sleep(1);
	}
	return 0;
}

