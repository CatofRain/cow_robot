#include "tty_uart_RW.h"

///*
//const char *device = "/dev/ttyCH9344USB0";
int speed = 4800;
int hardflow = 0;
int verbose = 1;
int flag_setZero=0;
int fd[8];
int prenum=0;
int curnum=0;
// */

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
 

 int libtty_close(int fd)
{
	return close(fd);
}
 

 int libtty_write(int fd)
{
	int nwrite;
	
	int i;
	char buf[6] = {0xAA, 0x2A, 0x7C, 0x1A, 0xBC, 0xF0 };
	char buf_setZero[18] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x0E, 0xAA, 0x2A, 0x7B, 0xFF, 0xE6, 0x48, 0xAA, 0x2A, 0x7C, 0x1A, 0xBC, 0xF0};
	
	if(flag_setZero)
	{	nwrite = write(fd, buf_setZero, sizeof(buf_setZero));
		flag_setZero--;
		 printf("wrote %d bytes already : %x %x... %x .\n", nwrite, buf_setZero[0],buf_setZero[1], buf_setZero[nwrite-1]);

	}

	else{
		nwrite = write(fd, buf, strlen(buf));
		printf("wrote %d bytes already : %x %x... %x .\n", nwrite, buf[0],buf[1], buf[nwrite-1]);
	}
	return nwrite;
}


 void sig_handler(int signo)
{
    printf("capture sign no:%d\n",signo);
	for(int i=0;i<8;i++){
		if (fd[i]!= 0) {
			int ret = libtty_close(fd[i]);
			if (ret != 0) {
				printf("libtty_close error.\n");
			}
			else
				printf("libtty_closed: %d.\n", i);
			}
	}
exit(0);
}

 int libtty_read(int fd)
{
	int nwrite, nread;
	char buf[N_read];
	int i;
	memset(buf,0,sizeof(N_read));
	nread = read(fd, buf, N_read);  // sizeof(buf)
	if (nread >= 0) {
		if(nread > N_read_out)
		printf("read nread %d bytes.\n", nread);
           
	} else {
		printf("read error: %d\n", nread);
		return nread;
	}
//*
	if (verbose && nread > N_read_out) {
		printf("*************************\n");
		for (i = 0; i < nread; i++)
			printf(" 0x%.2x", (uint8_t)buf[i]);
		printf("\n*************************\n");		
	}
	
	if(buf[0]==0xaa){
		curnum=buf[10]*256+buf[11];
	}else{
		curnum=buf[5]*256+buf[6];
	}
	return nread;
}


/*
int main(int argc, char *argv[])
{
	int ret;
	char c;
	unsigned long modemstatus;
	signal(SIGINT, sig_handler); 
	fd = libtty_open(device);
	if (fd < 0) {
		printf("libtty_open: %s error.\n", device);
		exit(0);
	}
	ret = libtty_setopt(fd, speed, 8, 1, 'n', hardflow);
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		exit(0);
	}
	while(1)
		{
			ret = libtty_write(fd);
			if (ret <= 0)
				printf("libtty_write error: %d\n", ret);
			sleep(2);
		}
	ret = libtty_close(fd);
	if (ret != 0) {
		printf("libtty_close error.\n");
		exit(0);
	}
}*/
