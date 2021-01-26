//gcc -o wpi wiringpi.c -lwiringPi
// home/pi/jiin/gps

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

int main ()
{
    int fd;
    char c;
    char buf[100];
    int cnt, i, cnt1;

    
    int LEN = 300;
    char Key[LEN];
    char *start_pt;

    int start;
    int comma[5];
    // Setup serial port on ODROID
    if ((fd = serialOpen ("/dev/ttyACM0",9600)) < 0) {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }
    if (wiringPiSetup () == -1) {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    //serialPrintf(fd,"\r"); // send enter key to read data from sensor
    delay(1000);

    

    while (1)//(serialDataAvail (fd)) 
    {
        //c = serialGetchar(fd);
        //printf("%c",c);

        start = 0;
        for(i=0;i<LEN;i++)
        {
            Key[i] = serialGetchar(fd);

            if((i<LEN-68)&&(i>5)&&(Key[i-5]=='$')&&(Key[i-1]=='G')&&(Key[i]=='A'))
            {
                start = i-5;
                *start_pt = Key[start];
                //printf("%c %c %c %c %c\n",Key[start+1],Key[start+2],Key[start+3],Key[start+4], Key[start+5]);
                

               // printf("%c %c %c %c %c %c %c %c %c %c \n",Key[start+17],Key[start+18],Key[start+19],Key[start+20], Key[start+21], 
                //Key[start+22], Key[start+23],Key[start+24],Key[start+25],Key[start+26]);
            }
            
        }
        //printf("%c \n",*start_pt);
        //printf("%c %c %c %c %c\n",Key[start+1],Key[start+2],Key[start+3],Key[start+4], Key[start+5]);


        if(start!=0)
        {
            //longitude
            printf("%c %c %c %c %c",Key[start+17],Key[start+18],Key[start+19],Key[start+20], Key[start+21]);
            printf("%c %c %c %c %c \n",Key[start+22], Key[start+23],Key[start+24],Key[start+25],Key[start+26]);

            //latitude
            printf("%c %c %c %c %c",Key[start+30],Key[start+31],Key[start+32],Key[start+33], Key[start+34]);
            printf("%c %c %c %c %c %c \n",Key[start+35], Key[start+36],Key[start+37],Key[start+38],Key[start+39],Key[start+40]);
            //printf("%c %c %c %c %c %c %c %c %c %c \n",Key[start+17],Key[start+18],Key[start+19],Key[start+20], Key[start+21], Key[start+22], Key[start+23],Key[start+24],Key[start+25],Key[start+26]);
        }
        
        
        delay(300);


        
        
    }

    serialClose(fd);    
}