/**
 * Template for Servo Control from FPGA with Hardware Controlled Speed
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#define BASE_ADDRESS 0x400D0000

//Servo motor offsets
#define Base_OFFSET 0x100
#define Bicep_OFFSET 0x104
#define Elbow_OFFSET 0x108
#define Wrist_OFFSET 0x10C
#define Gripper_OFFSET 0x110

#define REG_WRITE(addr, off, val) (*(volatile int*)(addr+off)=(val))

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
 * This function takes the servo number and the position, and writes the values in
 * appropriate address for the FPGA
 * @param test_base			base pointer for servos
 * @param servo_number		servo number to manipulate
 * @param position			new postion in degree (0 .. 180)
 * @param speed				speed to move in degree / 20ms
 */
void servo_move(unsigned char servo_number, unsigned char position, unsigned char speed);


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

	//Initialize all servo motors to middle position, go there fast
	servo_move(0, 150, 100);
	servo_move(1, 150, 100);
	servo_move(2, 150, 100);
	servo_move(3, 150, 100);
	servo_move(4, 150, 100);
	servo_move(5, 150, 100);

 	return 0;
}


/**
 * This function takes the servo number and the position, and writes the values in
 * appropriate address for the FPGA
 * @param test_base			base pointer for servos
 * @param servo_number		servo number to manipulate
 * @param position			new postion in degree (0 .. 180)
 * @param speed				speed to move in degree / 20ms
 */
void servo_move(unsigned char servo_number, unsigned char position, unsigned char speed) {

	/* writeValue bits 0..7    position
	 * 			  bits 8..15   speed
	 * 			  bits 16..31  all 0
	 */
	unsigned int writeValue = 0 << 16 | speed << 8 | position;

	switch (servo_number) {
        	case 1:  //Base
                	REG_WRITE(gServos.test_base, Base_OFFSET, writeValue);
                	break;

           	case 2:  //Bicep
                	REG_WRITE(gServos.test_base, Bicep_OFFSET, writeValue);
                	break;

          	case 3:  //Elbow
                	REG_WRITE(gServos.test_base, Elbow_OFFSET, writeValue);
                	break;

           	case 4:  //Wrist
                	REG_WRITE(gServos.test_base, Wrist_OFFSET, writeValue);
                	break;

           	case 5:  //Gripper
                	REG_WRITE(gServos.test_base, Gripper_OFFSET, writeValue);
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


int main()
{
	//Declarations and initialization
	int servo_number = 0;
	int position = 0;
  int speed = 0;

	printf("\n-------------  ATTENTION ROBOT WILL BE MOVING!  --------------------\n\n");
	printf("Please ensure robot power is OFF. Hold it in middle position. Then, turn it on.\n");
	sleep(1);

	/* initialize servos */
	if (servo_init() != 0) {
		return -1; // exit if init fails
	}
 
 servo_move(1, 140, 20);
 servo_move(2, 200, 20);
 servo_move(3, 160, 20);
 servo_move(5, 170, 20);
 sleep(1);
 servo_move(4, 110, 20);
 sleep(1);
 servo_move(5, 60, 20);
 sleep(1);
 //throw
 servo_move(2, 240, 50);
 servo_move(3, 240, 50);
 servo_move(4, 240, 50);
 servo_move(5, 240, 50);

/*
	do {
		printf("Enter servo number (1-5) or enter 0 to exit:\n");
    	scanf("%d", &servo_number); //Take the servo number from user

    	// if valid servo number
		if (servo_number != 0) {

			printf("Enter speed (1-50):\n");
			scanf("%d", &speed);  //Take the speed from user

			printf("Enter position (60 - 240):\n");
			scanf("%d", &position);  //Take the position from user

			//The selected servo will move to the desired position
			servo_move(servo_number, position, speed);
		}
	} while( servo_number != 0 ); // repeat while valid servo number given
 */

	/* deinitialize servos */
	servo_release();

	return 0;
}

