#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libgen.h>
#include <getopt.h>
#include <time.h>
#include <sched.h>
#include <limits.h>
#include <errno.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

//gcc -o wpi wiringpi.c -lwiringPi
#include <wiringPi.h>
#include <wiringSerial.h>

//#define CAN_MSG_ID	0x77
#define CAN_MSG_LEN	8
#define CAN_MSG_COUNT	50
#define CAN_MSG_WAIT	3
#define BUTPIN 7

FILE *fp;
FILE *logfp; 
static int running = 1;
static int verbose=1;
static int sockfd;
static int test_loops;

time_t timer;
char buffer[26];
struct tm* tm_info;

// gps variables
    char c, buf[100];
    int gfd, cnt, i, cnt1, start;    
    //int LEN = 300;
    char Key[300];
    char *start_pt;
    int comma[5];


/*
void button(void){

	printf("********\n");
	fp = popen("sudo /home/pi/linux-can-utils/cantest can0 015#1122334455667788", "r");
	shutdown(sockfd,SHUT_RDWR);
	running=0;

}
*/
static void echo_progress(unsigned char data)
{
	if (data == 0xff) {
		printf(".");
		fflush(stdout);
	}
}

static void signal_handler(int signo)
{
	close(sockfd);
	running = 0;
}

static int recv_frame(struct can_frame *frame)
{
	int ret;

	ret = recv(sockfd, frame, sizeof(*frame), 0);
	if (ret != sizeof(*frame)) {
		if (ret < 0)
			perror("recv failed");
		else
			fprintf(stderr, "recv returned %d", ret);
		return -1;
	}
	return 0;
}

static int send_frame(struct can_frame *frame)
{
	int ret;

	while ((ret = send(sockfd, frame, sizeof(*frame), 0))
	       != sizeof(*frame)) {
		if (ret < 0) {
			if (errno != ENOBUFS) {
				perror("send failed");
				return -1;
			} else {
				if (verbose) {
					printf("N");
					fflush(stdout);
				}
			}
		} else {
			fprintf(stderr, "send returned %d", ret);
			return -1;
		}
	}
	return 0;
}

static int can_echo_dut(void)
{
	struct can_frame frame;
	int i;

	while (running) {
		if (recv_frame(&frame)) {
			return -1;
		}
		
		//echo_progress(frame.data[0]);
		if(frame.can_id==0x111){  //use any id we want
			printf("\nCurrent SOC : %d%\n",frame.data[0]);			
		}
		
		if(frame.can_id==0x210){  
			printf("\nMotor Speed : %d \n",frame.data[1]);
			printf("\nWheel Angle : %d \n",frame.data[2]);	
			//printf("\nSOC : %d %\n",frame.data[3]);	
			/*
			if (frame.data[0] & 0x04 == 0x04){
				printf("\nBrake ON\n");	
			}else{
				printf("\nBrake OFF\n");
			}
			*/
		}

		//------------------gps data-------------------------//
		start = 0;
        for(i = 0; i < 300; i++)
        {
            Key[i] = serialGetchar(gfd);
            if((i<300-68) && (i>5) && (Key[i-5]=='$') && (Key[i-1]=='G') && (Key[i]=='A')){
                start = i-5;
                *start_pt = Key[start];
            }            
        }

		timer = time(NULL);
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d, %H:%M:%S", tm_info);
		fprintf(logfp, "%s %d %d ", buffer , frame.data[1], frame.data[2]);
        
        if(start!=0)
        {
            //latitude (ddmm.mmmmm)
            //printf("%c %c %c %c %c",Key[start+17],Key[start+18],Key[start+19],Key[start+20], Key[start+21]);
            //printf("%c %c %c %c %c \n",Key[start+22], Key[start+23],Key[start+24],Key[start+25],Key[start+26]);

            //longitude
            //printf("%c %c %c %c %c",Key[start+30],Key[start+31],Key[start+32],Key[start+33], Key[start+34]);
            //printf("%c %c %c %c %c %c \n",Key[start+35], Key[start+36],Key[start+37],Key[start+38],Key[start+39],Key[start+40]);
        
			
			//fprintf(logfp, "%c%c%c%c%c", Key[start+17],Key[start+18],Key[start+19],Key[start+20], Key[start+21]);
			//fprintf(logfp, "%c%c%c%c%c", Key[start+22], Key[start+23],Key[start+24],Key[start+25],Key[start+26]);

			//fprintf(logfp, "%c%c%c%c%c", Key[start+30],Key[start+31],Key[start+32],Key[start+33], Key[start+34]);
			//fprintf(logfp, "%c%c%c%c%c%c\n", Key[start+35], Key[start+36],Key[start+37],Key[start+38],Key[start+39],Key[start+40]);		
		}
	}
	return 0;
}

int main()
{
	int state;
	struct ifreq ifr;
	struct sockaddr_can addr;
	char *intf_name;
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
	int echo_gen = 0;
	int opt, err;

	//time(&t);
	logfp = fopen("/home/pi/jiin/canfdtest/canlog2.txt", "w");
	if (logfp == NULL){
		printf("error\n");
	}
	//fclose(logfp);

	fp = popen("sudo /sbin/ip link set can0 up type can bitrate 500000", "r");
	if(fp==NULL)
	{
		perror("erro : ");
		exit(0);
	}
	state = pclose(fp);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	intf_name = "can0";

	// Setup serial port on ODROID
    if ((gfd = serialOpen ("/dev/ttyACM0",9600)) < 0) {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }
    if (wiringPiSetup () == -1) {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    //serialPrintf(gfd,"\r"); // send enter key to read data from sensor
    delay(1000);

	printf("interface = %s, family = %d, type = %d, proto = %d\n",
	       intf_name, family, type, proto);

	if ((sockfd = socket(family, type, proto)) < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = family;
	strcpy(ifr.ifr_name, intf_name);
	ioctl(sockfd, SIOCGIFINDEX, &ifr);
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(sockfd);
		return 1;
	}

	err = can_echo_dut();

	if (verbose){
		printf("Exiting...\n");}

	serialClose(gfd);   
	close(sockfd);
     

	return err;

}
