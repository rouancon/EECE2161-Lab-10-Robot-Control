/**
 * Template for Servo Control from FPGA with Hardware Controlled Speed
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


/************ SERVO CONSTANTS ************/

#define BASE_ADDRESS 0x400D0000

//Servo motor offsets
#define Base_OFFSET 0x100
#define Bicep_OFFSET 0x104
#define Elbow_OFFSET 0x108
#define Wrist_OFFSET 0x10C
#define Gripper_OFFSET 0x110

#define REG_WRITE(addr, off, val) (*(volatile int*)(addr+off)=(val))


/************ WIIMOTE CONSTANTS ****************/

/** Event 2 is of 32 chars in size*/
#define WIIMOTE_EVT2_PKT_SIZE 32

/** device file name for event 2	*/
#define WIIMOTE_EVT2_FNAME "/dev/input/event2"

/** the code is placed in byte 10 */
#define WIIMOTE_EVT2_CODE 10
/** value is placed in byte 12	*/
#define WIIMOTE_EVT2_VALUE 12


/** Event 2 is of 32 chars in size*/
#define WIIMOTE_EVT0_PKT_SIZE 16

/** device file name for event 2	*/
#define WIIMOTE_EVT0_FNAME "/dev/input/event0"

/** the code is placed in byte 10 */
#define WIIMOTE_EVT0_CODE 10

/** code for X acceleration */
#define WIIMOTE_EVT0_ACCEL_X 3
/** code for Y acceleration */
#define WIIMOTE_EVT0_ACCEL_Y 4
/** code for Z acceleration */
#define WIIMOTE_EVT0_ACCEL_Z 5

/** high portion of value is placed in byte 13	*/
#define WIIMOTE_EVT0_VALUE_H 13

/** high portion of value is placed in byte 12	*/
#define WIIMOTE_EVT0_VALUE_L 12



/**************** SERVO TYPES ****************/

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


/************ WIIMOTE TYPES ****************/

/**
 * WiiMote Button Codes
 */
typedef enum {
      UP = 1,
      DOWN,
      LEFT,
      RIGHT,
      A,
      PLUS,
      MINUS,
      HOME,
      ONE,
      TWO,
      B
 } buttonCode;


/**
 * WiiMote Button Event
 */
typedef struct {
	buttonCode code;  /// event 2 code
	unsigned char value; /// event 2 value

} tWiiMoteButton;

/**
 * WiiMote accelerator Event
 */
typedef struct {
	unsigned char code;  /// event 0 code
	signed short value; /// event  0 value
} tWiiMoteAccel;


/**
 * structure for WiiMote object
 */
typedef struct {
	int fileEvt2; // file desriptor for event 2
	int fileEvt0; // file descriptor for event 0
} tWiiMote;


/** instantiate one WiiMote object. It is assumed to be a singleton
 * for this application (i.e. exactly one instance)
 */
tWiiMote gWiiMote;


/***************** SERVO FUNCTIONS *************/

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
  servo_move(0, 150, 100);
	servo_move(1, 150, 100);
	servo_move(2, 150, 100);
	servo_move(3, 150, 100);
	servo_move(4, 150, 100);
	servo_move(5, 150, 100);
	// Releasing the mapping in memory
	munmap((void *)gServos.test_base, gServos.map_len);
	close(gServos.fd);
}




/************ WIIMOTE FUNCTIONS ****************/

/**
 * Initialize WiiMote accelerometer and button read
 * @return 0 on success, != 0 otherwise.
 */
int wiimote_init(void){
	//Opens the event2 file in read only mode
	// also set O_NONBLOCK which makes read calls non blocking, i.e. if less data is available
	// than asked for, the read call returns and errno is set to EWOULDBLOCK
	gWiiMote.fileEvt2 = open(WIIMOTE_EVT2_FNAME , O_RDONLY | O_NONBLOCK);

	// failed to open file?
	if (gWiiMote.fileEvt2 == -1) {
		// error handling, print error and exit with code
		printf("Could not open event file '%s'\n", WIIMOTE_EVT2_FNAME);
		return -1;
	}


	// open file for accelerometer -- blocking
	gWiiMote.fileEvt0 = open(WIIMOTE_EVT0_FNAME , O_RDONLY); //Opens the event2 file in read only mode

	// failed to open file?
	if (gWiiMote.fileEvt0  == -1) {
		// error handling, print error and exit with code
		printf("Could not open event file '%s'\n", WIIMOTE_EVT0_FNAME);
		return -1;
	}


	return 0;
}

/**
 * get event codes for wii buttons
 *
 * @return button even, if no button code detected return (0,0)
 */
tWiiMoteButton wiimote_buttonGet(){
	unsigned char buf[WIIMOTE_EVT2_PKT_SIZE]; // allocate data for packet buffer
	tWiiMoteButton button;

	// start out with nothing received
	button.code = 0;
	button.value = 0;

	// blocking read of  32 bytes from  file and put them into buf
	// Without O_NONBLOCK read blocks (does not return) until sufficient bytes are available
	// with O_NONBLOCK read immediately returns even if fewer bytes than asked for are availble, but it sets
	// errno then.

	// only continue if we got all bytes asked for.
	if (WIIMOTE_EVT2_PKT_SIZE == read(gWiiMote.fileEvt2, buf, WIIMOTE_EVT2_PKT_SIZE)) {


		// extract code from packet and set button.code accordingly
   switch (buf[WIIMOTE_EVT2_CODE]) {
   case 0x30:
        button.code = A;
        break;
   case 0x31:
        button.code = B;
        break;
   case 0x1:
        button.code = ONE;
        break;
   case 0x2:
        button.code = TWO;
        break;
   case 0x6C:
        button.code = DOWN;
        break;
   case 0x3C:
        button.code = HOME;
        break;
   default:
        break;
   }

		// extract value from packet
		button.value = buf[WIIMOTE_EVT2_VALUE];

	} else {
		// got fewer bytes ,,,
		// if error is different than it would block then report
		if (errno != EWOULDBLOCK) {
			printf("Unknown error %d\n", errno);
		}
	}

	// return received button code (or none if no code received)
	return button;

}

/**
 * get acceleration events from wiimote
 * @return acceleration event
 */
tWiiMoteAccel wiimote_accelGet() {
	unsigned char buf[WIIMOTE_EVT0_PKT_SIZE]; //each packet of data is 16 bytes
	unsigned char evt0ValueL; /// event 0 value Low
	unsigned char evt0ValueH; /// event 0 value Low
	tWiiMoteAccel accel;

	accel.code = 0; // start out with nothing received

	// read 16 bytes from the file and put it in the buffer
	read(gWiiMote.fileEvt0, buf, WIIMOTE_EVT0_PKT_SIZE);

	accel.code = buf[WIIMOTE_EVT0_CODE];       // extract code byte

	// if we got an accel change
	if (accel.code != 0) {
		evt0ValueH = buf[WIIMOTE_EVT0_VALUE_H];  // extract high byte of accel
		evt0ValueL = buf[WIIMOTE_EVT0_VALUE_L];  // extract low byte of accel
		accel.value = evt0ValueH << 8 | evt0ValueL;
	}

	// return read accel up (may be with code 0).
	return accel;
}


/**
 * close the wiimote connection
 */
void wiimote_close() {
	close(gWiiMote.fileEvt0); // close acceleration file
	close(gWiiMote.fileEvt2); // close button file
}


/************** MAIN ***********************/


int main()
{
	//Servo variables
	int servo_number = 0;
	long position = 0;
  int prevPosn = 150;
  int speed = 10;
  
  //Wiimote variables
  tWiiMoteButton button;
	tWiiMoteAccel accel;
	unsigned char accelX =0;
	unsigned char accelY =0;
 
  int buttonValue = 0;
 
  // Initialize wiimote
  if (wiimote_init() != 0) {
		printf("Failed to init WiiMote\n");
		return -1;
	}

	printf("\n-------------  ATTENTION ROBOT WILL BE MOVING!  --------------------\n\n");
	printf("Please ensure robot power is OFF. Hold it in middle position. Then, turn it on.\n");
	sleep(1);

	/* initialize servos */
	if (servo_init() != 0) {
		return -1; // exit if init fails
	}
 

 
 do {
		// read acceleration (blocking)
		accel = wiimote_accelGet();
		// read button events that have accumulated since then.
		button = wiimote_buttonGet();

   position = ((accel.value * 18) / 1000) + 150;
   
 		switch (button.code) {
		case A:
			servo_number = 1;
      buttonValue = 1;
			break;
		case B:
			servo_number = 2;
      buttonValue = 1;
			break;
    case ONE:
			servo_number = 3;
      buttonValue = 1;
			break;
    case TWO:
			servo_number = 4;
      buttonValue = 1;
			break;
    case DOWN:
			servo_number = 5;
      buttonValue = 1;
			break;
    default:
      buttonValue = 0;
      break;
		}
   
		// did we get an accel event?
		if (accel.code != 0) { // ignore the zeroes
			if( buttonValue && (accel.code == WIIMOTE_EVT0_ACCEL_X)) {
				servo_move(servo_number, prevPosn += position, speed);
        //printf("%d, %d \n", position, servo_number);
			}
		}


	// repeat until "Home" button is pressed (or relased)
	} while(button.code != HOME);
 
 wiimote_close();
 servo_release();
 return 0;
}
