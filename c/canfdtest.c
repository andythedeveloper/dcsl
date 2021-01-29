///home/pi/jiin/canfdtest/canfdtest.c
// Updated on Jan 29th

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libgen.h>
#include <getopt.h>
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



FILE *fp;
FILE *logfp; 
static int running = 1;
static int verbose=1;
static int sockfd;

time_t timer;
struct tm* tm_info;
char buffer[26]; //CAN Bus buffer

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

static int can_echo(void)
{
	struct can_frame frame;
	


	while (running) {
		if (recv_frame(&frame)) {
			return -1;
		}
		
		if(frame.can_id == 0x111){  \
			printf("\nCurrent SoC : %d %c \n",frame.data[0], '%');			
		}
		
		if(frame.can_id == 0x210){  
			printf("\nMotor Speed : %d \n",frame.data[1]);
			printf("\nWheel Angle : %d \n",frame.data[2]);	
			//printf("\nSoC : %d %\n",frame.data[3]);	
			/*
     			if (frame.data[0] & 0x04 == 0x04){
				printf("\nBrake ON\n");	
			}else{
				printf("\nBrake OFF\n");
			}
			*/
		}

		// timestamp
		timer = time(NULL);
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d, %H:%M:%S", tm_info);	
        
		// log car status data into canlog.txt
		fprintf(logfp, "%s %d %d \n", buffer , frame.data[1], frame.data[2]);

        
	}
	return 0;
}



int main()
{
	// can variable
	struct ifreq ifr;
	struct sockaddr_can addr;
	char *intf_name;
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
    int err;
    //int state;

	logfp = fopen("/home/pi/jiin/canfdtest/canlog.txt", "w");
	if (logfp == NULL){
		printf("error\n");
	}

	fp = popen("sudo /sbin/ip link set can0 up type can bitrate 500000", "r");
	if(fp==NULL)
	{
		perror("erro : ");
		exit(0);
	}
	//state = pclose(fp);
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

	err = can_echo();

	if (verbose)
		printf("Exiting...\n");

	close(sockfd); //can

	return err;

}
