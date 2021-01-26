///home/pi/jiin/canfdtest/canfdtest.c

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
			printf("\nCurrent SOC : %d %\n",frame.data[0]);			
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

		timer = time(NULL);
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d, %H:%M:%S", tm_info);
	
		fprintf(logfp, "%s %d %d \n", buffer , frame.data[1], frame.data[2]);


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

	if (verbose)
		printf("Exiting...\n");

	close(sockfd);

	return err;

}