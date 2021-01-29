//gcc -o wpi wiringpi.c -lwiringPi
// Updated on Jan 29th

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

int main ()
{
    static int LEN = 300; //#define LEN 300 cause segmentation fault
    char c, buf[100];
    int gfd, i, start;    
    char key[LEN];
    char *start_pt;

    if ((gfd = serialOpen ("/dev/ttyACM0",9600)) < 0) {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }
    if (wiringPiSetup () == -1) {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    delay(1000);

    while (1) {

        start = 0;

        for(i=0; i<LEN; i++)
        {
            key[i] = serialGetchar(gfd); //(serialDataAvail (gfd)) 

            if((i<LEN-68) && (i>5) && (key[i-5]=='$') && (key[i-1]=='G') && (key[i]=='A')){
                start = i-5;
                *start_pt = key[start];
            }            
        }
        
        if(start != 0)
        {
            //latitude (ddmm.mmmmm)
            // from key[start+17] to key[start+26]
            printf("\n latitude: ");
            for (i=17; i<27; i++){
                printf("%c", key[start+i]);
            }         
            
            //longitude (ddmm.mmmmm)
            // from key[start+30] to key[start+40]
            printf("\n longitude: ");
            for (i=30; i<41; i++){
                printf("%c", key[start+i]);
            }
        }
        delay(300);        
    }

    serialClose(gfd);    
}
