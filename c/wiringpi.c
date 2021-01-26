//gcc -o wpi wiringpi.c -lwiringPi
// home/pi/jiin/gps

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

int main ()
{
    char c, buf[100];
    int gfd, cnt, i, cnt1, start;    
    int LEN = 300;
    char key[LEN];
    char *start_pt;
    int comma[5];

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

    

    while (1)//(serialDataAvail (gfd)) 
    {

        start = 0;
        for(i=0;i<LEN;i++)
        {
            key[i] = serialGetchar(gfd);

            if((i<LEN-68)&&(i>5)&&(key[i-5]=='$')&&(key[i-1]=='G')&&(key[i]=='A'))
            {
                start = i-5;
                *start_pt = key[start];
                }
            
        }
        
        if(start!=0)
        {
            //latitude (ddmm.mmmmm)
            // from key[start+17] to key[start+26]
            printf("\n");
            for (i=17; i<27; i++){
                printf("%c", key[start+i]);
            }         
            
            //longitude (ddmm.mmmmm)
            // from key[start+30] to key[start+40]
            printf("\n");
            for (i=30; i<41; i++){
                printf("%c", key[start+i]);
            }
        }
        
        delay(300);        
    }

    serialClose(gfd);    
}
