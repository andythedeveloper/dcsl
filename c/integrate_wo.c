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

// serial communication
#include <wiringPi.h>
#include <wiringSerial.h>

//#define LEN 300 //segmentation fault

FILE *fp;
FILE *logfp; 
static int running = 1;
static int verbose=1;
static int sockfd;
//static int LEN = 300;   //variably modified ‘key’ at file scope

time_t timer;
struct tm* tm_info;
char buffer[26]; //CAN Bus buffer

// gps variables
char c, buf[100];
char latitude[10], longitude[11];
int gfd, i, j, start; 
char *start_pt;

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
	
	// gps buffer variables
	static int LEN = 300;   
	char key[LEN];
	//char res;

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
			
     		//	if (frame.data[0] & 0x04 == 0x04){
			//	printf("\nBrake ON\n");	
			//}else{
			//	printf("\nBrake OFF\n");
			//}
			
		}
	
		// timestamp
		timer = time(NULL);
		tm_info = localtime(&timer);
		strftime(buffer, 26, "%Y-%m-%d, %H:%M:%S", tm_info);	
        
		// log car status data into canlog.txt
		fprintf(logfp, "%s %d %d ", buffer , frame.data[1], frame.data[2]);
		
		//---------------------------------------gps---------------------------------------
		start = 0;
		
        for(i = 0; i < LEN; i++){
            key[i] = serialGetchar(gfd);
			
			// filter gps data that starts with $GNGGA
            if((i<LEN-68) && (i>5) && (key[i-5]=='$') && (key[i-1]=='G') && (key[i]=='A')){
                start = i-5;
               // *start_pt = key[start];
            }            
        }
		
		 if(start != 0)
        {
            // for loop took too much time and caused segmentation fault
			
			//latitude (ddmm.mmmmm)
			printf("latitude: ");
			for (i=17; i<27; i++){
                printf("%c", key[start+i]);
            }    

            //printf("%c %c %c %c %c",key[start+17],key[start+18],key[start+19],key[start+20], key[start+21]);
            //printf("%c %c %c %c %c \n",key[start+22], key[start+23],key[start+24],key[start+25],key[start+26]);

            //longitude
			printf("\nlongitude: ");
			for (i=30; i<41; i++){
                printf("%c", key[start+i]);
            }
        	//printf("%c %c %c %c %c",key[start+30],key[start+31],key[start+32],key[start+33], key[start+34]);
            //printf("%c %c %c %c %c %c \n",key[start+35], key[start+36],key[start+37],key[start+38],key[start+39],key[start+40]);



			fprintf(logfp, "%c%c%c%c%c", key[start+17],key[start+18],key[start+19],key[start+20], key[start+21]);
			fprintf(logfp, "%c%c%c%c%c ", key[start+22], key[start+23],key[start+24],key[start+25],key[start+26]);

			fprintf(logfp, "%c%c%c%c%c", key[start+30],key[start+31],key[start+32],key[start+33], key[start+34]);
			fprintf(logfp, "%c%c%c%c%c%c\n", key[start+35], key[start+36],key[start+37],key[start+38],key[start+39],key[start+40]);	

			// save location
			for (i=0; i<10; i++){
				for(j=17; j<27; j++){
					latitude[i] = key[start+j];
				}
			}

			for (i=0; i<11; i++){
				for(j=30; j<41; j++){
					longitude[i] = key[start+j];
				}
			}			

			/*
			latitude[0] = key[start+17];
			latitude[1] = key[start+18];
			latitude[2] = key[start+19];
			latitude[3] = key[start+20];
			latitude[4] = key[start+21];
			latitude[5] = key[start+22];
			latitude[6] = key[start+23];
			latitude[7] = key[start+24];
			latitude[8] = key[start+25];
			latitude[9] = key[start+26];
			*/

			// save longitude
			/*
			longitude[0] = key[start+30];
			longitude[1] = key[start+31];
			longitude[2] = key[start+32];
			longitude[3] = key[start+33];
			longitude[4] = key[start+34];
			longitude[5] = key[start+35];
			longitude[6] = key[start+36];
			longitude[7] = key[start+37];
			longitude[8] = key[start+38];
			longitude[9] = key[start+39];
			longitude[10] = key[start+40];
			*/



		} else {	
			// print previous location
			
			fprintf(logfp, "%c%c%c%c%c", latitude[0], latitude[1], latitude[2], latitude[3], latitude[4]);
			fprintf(logfp, "%c%c%c%c%c ", latitude[5],latitude[6] , latitude[7], latitude[8], latitude[9]);

			fprintf(logfp, "%c%c%c%c%c", longitude[0], longitude[1], longitude[2], longitude[3], longitude[4]);
			fprintf(logfp, "%c%c%c%c%c%c\n", longitude[5], longitude[6], longitude[7], longitude[8], longitude[9], longitude[10]);	


			
		}

        //delay(300);  
        
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

    //---------------------------gps setup start-----------------------//
    // Setup serial port on ODROID, baudrate = 9600
    if ((gfd = serialOpen ("/dev/ttyACM0",9600)) < 0) {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }
    if (wiringPiSetup () == -1) {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    //---------------------------gps setup end-----------------------//


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
	
    serialClose(gfd); //gps   

	return err;

}
