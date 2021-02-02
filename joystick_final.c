/**
 * Reference Author: Jason White
 *
 * Editor: Jiin Sang. Duc 
 *
 * https://www.kernel.org/doc/Documentation/input/joystick-api.txt
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <stdlib.h>
#include <string.h>

#define MAXCHAR 10 //canlog

int state; // can state
FILE *fp;

// canlog variable
FILE *canfp;
FILE *canlogfp;
//char canbyte;
char str[MAXCHAR];
char* filename = "/home/pi/jiin/canlog.txt";


char des_angle_str[3]={'0', '0'};
char VCUcommand[33]={'c','a','n','s','e','n','d',' ','c','a','n','0',' ','2','0','0','#','8','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};//33
 /*                   0                   5                   10                  15      17  18  19  20  21                                          32         */
unsigned int des_angle = 90;
/**
 * Reads a joystick event from the joystick device.
 *
 * Returns 0 on success. Otherwise -1 is returned.
 */
int read_event(int fd, struct js_event *event)
{
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    /* Error, could not read full event. */
    return -1;
}

/**
 * Returns the number of axes on the controller or 0 if an error occurs.
 */
size_t get_axis_count(int fd)
{
    __u8 axes;

    if (ioctl(fd, JSIOCGAXES, &axes) == -1)
        return 0;

    return axes;
}

/**
 * Returns the number of buttons on the controller or 0 if an error occurs.
 */
size_t get_button_count(int fd)
{
    __u8 buttons;
    if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1)
        return 0;

    return buttons;
}

/**
 * Current state of an axis.
 */
struct axis_state {
    short x, y;
};

/**
 * Keeps track of the current axis state.
 *
 * NOTE: This function assumes that axes are numbered starting from 0, and that
 * the X axis is an even number, and the Y axis is an odd number. However, this
 * is usually a safe assumption.
 *
 * Returns the axis that the event indicated.
 */
size_t get_axis_state(struct js_event *event, struct axis_state axes[3])
{
    size_t axis = event->number / 2;

    if (axis < 3)
    {
        if (event->number % 2 == 0)
            axes[axis].x = event->value;
        else
            axes[axis].y = event->value;
    }

    return axis;
}

int main(int argc, char *argv[])
{
    const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;

    if (argc > 1)
        device = argv[1];
    else
        device = "/dev/input/js0";

    js = open(device, O_RDONLY);

    if (js == -1)
        perror("Could not open joystick");
    
    //---------can initial setup start-----------------//
    fp = popen("sudo /sbin/ip link set can0 up type can bitrate 500000", "r");
	if(fp==NULL)
	{
		perror("error opening can port : ");
		exit(0);
	}
	state = pclose(fp);
    //---------can initial setup end-----------------//

    // candump -tA can0,00200200:1fffffff,210:7ff
    // popen() returns a pointer to an open stream that can be used to read or write to the pipe
    //canfp = popen("candump can0,00200200:1fffffff,210:7ff >> canlog.txt", "r"); 
    canfp = popen("candump can0 >> canlog.txt", "r"); 
    
    if(canfp==NULL){
		perror("error logging the can data : ");
		exit(0);
	}

    canlogfp = fopen(filename, "r");
    if (canlogfp == NULL){
        printf("Could not open file %s",filename);
        return 1;
    }
    
    
    




    // -------------------------remote control-------------------------//
    /* This loop will exit if the controller is unplugged. */
    while (1)
    {
        // can log
        /*
        while (canbyte = fgetc(canlogfp) != EOF) {
            printf("%c", canbyte);
        }*/
        /*
        canbyte = fgetc(canlogfp);
        while (canbyte = fgetc(canlogfp) != EOF) {
            printf("%c", canbyte);
        }
        */

        fseek(canlogfp, 0L, SEEK_END);
        int count = ftell(canlogfp);

        while (fgets(str, MAXCHAR, canlogfp) != NULL) {
            //printf("%s", str);
            printf(count);
        }
        
        //joystick event 
        while (read_event(js, &event) == 0) {            
            switch (event.type)
            {
            case JS_EVENT_BUTTON:
                printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
                
                switch (event.number)
                {
                    int i;
                    
                    // BACK = 6 : brake on
                    case 6:
                        // sets brake on
                        VCUcommand[17] = 'C';
                        VCUcommand[18] = '0';
                        
                        // sets speed to 0
                        VCUcommand[19] = '0';
                        VCUcommand[20] = '0';
                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);              
                    break;
                    
                    // START = 7 : brake off
                    case 7:
                        VCUcommand[17] = 'A';
                        VCUcommand[18] = '0';
                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;
        

                    //  X : enabling remote control
                    case 2: 
                        for(i=0; i<6; i++) {
                            fp = popen("cansend can0 200#8000000000000000", "r");
                            printf("%s\n", VCUcommand);
                            state = pclose(fp);
                        }
                    break;

                    // Y : forward control
                    // cansend can0 200# 80 05 00 00 00 00 00 00
                    case 3: 
                        VCUcommand[19] = '0';
                        VCUcommand[20] = '5';
                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;
                    
                    // A : backward control
                    // cansend can0 200#800FB00000000000 (fb = -5 in signed 2's complement)
                    case 0:  
                        VCUcommand[19] = 'F';
                        VCUcommand[20] = 'B';
                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;
                    
                    // B : stop
                    // cansend can0 200#8000000000000000
                    case 1: 
                        VCUcommand[19] = '0';
                        VCUcommand[20] = '0';
                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;

                    // 90 degree: straight, min 40 degree, max 130 degree
                    // LB = 4 : turn left (40~130)
                    // cansend can0 200# 80 00 00 05 00 00 00 00
                    case 4: 
                        if((des_angle-5) >= 40 && (des_angle-5) <= 130){
                            des_angle = des_angle - 5;
                        }

                        //change integer to string
                        sprintf(des_angle_str, "%x", des_angle); 

                        //assign value
                        if(des_angle_str[1] == '\0'){
                            VCUcommand[21] = '0';
                            VCUcommand[22] = *des_angle_str;
                        }
                        else if(des_angle_str[0] == '\0'&& des_angle_str[1] == '\0'){
                            VCUcommand[21] = '0';
                            VCUcommand[22] = '0';
                        }
                        else{
                            VCUcommand[21] = *des_angle_str;
                            VCUcommand[22] = *(des_angle_str+1);
                        }

                        // cansend can0 200#8200000000000000
                        if (des_angle < 85){
                            VCUcommand[18] = '2';
                        }
                        // cansend can0 200#8100000000000000 
                        else if (des_angle > 95) {
                            VCUcommand[18] = '1';
                        }
                        else {
                            VCUcommand[18] = '0';
                        }

                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;
                    
                    //RB = 5 : turn right (90~130)
                    case 5: 
                        if((des_angle+5)>=40&&(des_angle+5)<=130){
                            des_angle = des_angle + 5;}

                        sprintf(des_angle_str, "%x", des_angle);

                        //assign value
                        if(des_angle_str[1] == '\0'){
                            VCUcommand[21] = '0';
                            VCUcommand[22] = *des_angle_str;
                        }
                        else if(des_angle_str[0] == '\0'&& des_angle_str[1] == '\0'){
                            VCUcommand[21] = '0';
                            VCUcommand[22] = '0';
                        }
                        else{
                            VCUcommand[21] = *des_angle_str;
                            VCUcommand[22] = *(des_angle_str+1);
                        }

                        // cansend can0 200#8200000000000000
                        if (des_angle < 85){
                            VCUcommand[18] = '2';
                        }
                        // cansend can0 200#8100000000000000 
                        else if (des_angle > 95) {
                            VCUcommand[18] = '1';
                        }
                        else {
                            VCUcommand[18] = '0';
                        }

                        fp = popen(VCUcommand, "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;

                    // Logo: Disable Remote Control
                    case 8: 
                        fp = popen("cansend can0 200#0000000000000000", "r");
                        printf("%s\n", VCUcommand);
                        state = pclose(fp);
                    break;
                }
                break;

            case JS_EVENT_AXIS:
                axis = get_axis_state(&event, axes);
                if (axis < 3)
                    printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
                break;
            default:
                /* Ignore init events. */
                break;
        }
            fflush(stdout);
        }
        
    }

    close(js);

    return 0;
}
