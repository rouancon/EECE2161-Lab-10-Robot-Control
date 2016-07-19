/**
 * Template for Servo Control from FPGA with Software Controlled Speed
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define BASE_ADDRESS 0x400D0000

//Servo motor offsets
#define Base_OFFSET 0x100
#define Bicep_OFFSET 0x104
#define Elbow_OFFSET 0x108
#define Wrist_OFFSET 0x10C
#define Gripper_OFFSET 0x110

#define REG_WRITE(addr, off, val) (*(volatile int*)(addr+off)=(val))
#define REG_READ(addr, off) (*(volatile int*)(addr+off))

/**
 * data structure for servo instance
 */
typedef struct {
	unsigned char *test_base; /// base address of mapped virtual space
	int fd;                   /// file desrcriptor for memory map
	int map_len;              /// size of mapping window

} tServo;


/**
 * global variable for all servos
 */
tServo gServos;

/**
 * Initialize servos
 * @return 0 upon success, 1 otherwise
 */
int servo_init() {

	//Open the file regarding memory mapped IO to write values for the FPGA	
	gServos.fd = open( "/dev/mem", O_RDWR);
	
	unsigned long int PhysicalAddress = BASE_ADDRESS;
	gServos.map_len= 0xFF;  //size of mapping window

	// map physical memory startin at BASE_ADDRESS into own virtual memory
	gServos.test_base = (unsigned char*)mmap(NULL, gServos.map_len, PROT_READ | PROT_WRITE, MAP_SHARED, gServos.fd, (off_t)PhysicalAddress);

	// did it work?
	if(gServos.test_base == MAP_FAILED)	{
		perror("Mapping memory for absolute memory access failed -- Test Try\n");
		return 1;
	}

	//Initialize all servo motors
	// I assume this is the "sleep" position
	REG_WRITE(gServos.test_base, Base_OFFSET, 150);
	REG_WRITE(gServos.test_base, Bicep_OFFSET, 190);
	REG_WRITE(gServos.test_base, Elbow_OFFSET, 190);
	REG_WRITE(gServos.test_base, Wrist_OFFSET, 100);
 	REG_WRITE(gServos.test_base, Gripper_OFFSET, 150);

 	return 0;
}

/**
 * This function takes the servo number and the position, and writes the values in
 * appropriate address for the FPGA
 * @param test_base			base pointer for servos
 * @param servo_number		servo number to manipulate
 * @param position			new postion
 */
void servo_move(int servo_number, int position) {
	switch (servo_number) {
        	case 1:  //Base
                	REG_WRITE(gServos.test_base, Base_OFFSET, position);
                	break;

           	case 2:  //Bicep
                	REG_WRITE(gServos.test_base, Bicep_OFFSET, position);
                	break;

          	case 3:  //Elbow
                	REG_WRITE(gServos.test_base, Elbow_OFFSET, position);
                	break;

           	case 4:  //Wrist
                	REG_WRITE(gServos.test_base, Wrist_OFFSET, position);
                	break;

           	case 5:  //Gripper
                	REG_WRITE(gServos.test_base, Gripper_OFFSET, position);
                	break;

           	default:
                	break;
	}
}

/**
 * Deinitialize Servos
 */
void servo_release(){
	// Releasing the mapping in memory
	munmap((void *)gServos.test_base, gServos.map_len);
	close(gServos.fd);
}

/**
 * Move Servo given a speed.
 * @param servoNr		selected servo number
 * @param from			start position (0-180)
 * @param to			end position (0-180)
 * @param speed			speed (degree/sec) >0
 */
void servoMove(unsigned int servoNr, int from, int to, int speed)
{
   int numPeriods = (abs(to - from) / speed) * 50000;
   float increment = (float)(to - from) / (float)numPeriods;
   
	
	switch (servoNr) {
        	case 1:  //Base
        			for (int i = 0; i < numPeriods; ++i) {
                		REG_WRITE(gServos.test_base, Base_OFFSET, (int)(from + (increment * i)));
                	}
                	break;

           	case 2:  //Bicep
           			for (int i = 0; i < numPeriods; ++i) {
                		REG_WRITE(gServos.test_base, Bicep_OFFSET, (int)(from + (increment * i)));
                	}
                	break;

          	case 3:  //Elbow
          			for (int i = 0; i < numPeriods; ++i) {
                		REG_WRITE(gServos.test_base, Elbow_OFFSET, (int)(from + (increment * i)));
                	}
                	break;

           	case 4:  //Wrist
           			for (int i = 0; i < numPeriods; ++i) {
                		REG_WRITE(gServos.test_base, Wrist_OFFSET, (int)(from + (increment * i)));
                   printf("%d, %d, %d, %f\n", (int)(from + (increment * i)), numPeriods, i, increment);
                	}
                	break;

           	case 5:  //Gripper
           			for (int i = 0; i < numPeriods; ++i) {
                		REG_WRITE(gServos.test_base, Gripper_OFFSET, (int)(from + (increment * i)));
                	}
                	break;

           	default:
                	break;
	}
}


int main()
{
	//Declarations and initialization
	int servo_number = 0;
  int lp_base = 150, lp_bicep = 190, lp_elbow = 190, lp_wrist = 100, lp_gripper = 190;
  int speed, newPosn;

	printf("\n-------------  Robot TESTING  --------------------\n\n");

	/* initialize servos */
	if (servo_init() != 0) {
		return -1; // exit if init fails
	}

	do {
		printf("Enter servo number (1-5) or enter 0 to exit:\n");
		scanf("%d", &servo_number); //Take the servo number from user

		if (servo_number != 0) {

    		printf("Enter position (60 - 240):\n");
    		scanf("%d", &newPosn); //Take the position from user
        printf("Enter speed (deg/sec) (1-90):\n");
    		scanf("%d", &speed); //Take the speed from user
               
       
       switch (servo_number) {
              case 1: 
                   servoMove(1, lp_base, newPosn, speed);
                   lp_base = newPosn;
                   break;
              case 2: 
                   servoMove(2, lp_bicep, newPosn, speed);
                   lp_bicep = newPosn;
                   break;
              case 3: 
                   servoMove(3, lp_elbow, newPosn, speed);
                   lp_elbow = newPosn;
                   break;
              case 4: 
                   servoMove(4, lp_wrist, newPosn, speed);
                   lp_wrist = newPosn;
                   break;
              case 5: 
                   servoMove(5, lp_gripper, newPosn, speed);
                   lp_gripper = newPosn;
                   break;
              default: break;
       }
		}
	} while( servo_number != 0); // repeat while valid servo number given

	/* deinitialize servos */
	servo_release();

	return 0;
}

