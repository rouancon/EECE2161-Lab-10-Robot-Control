#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
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
 * Move Servo given a speed.
 * @param servoNr		selected servo number
 * @param from			start position (0-180)
 * @param to			end position (0-180)
 * @param speed			speed (degree/sec) >0
 */
void servoMove(unsigned int servoNr, int from, int to, int speed)
{
	int startCyc = (10*from) + 600;
  	int endCyc = (10*to) + 600;
  	
  	int numPeriods = (abs((to - from)) / speed) * 50;
  	int increment = (endCyc - startCyc) / numPeriods;
	
	switch (servoNr) {
        	case 0:  //Base
        			for (int i = 0; i < numPeriods; ++i) {
                		//REG_WRITE(gServos.test_base, Base_OFFSET, (startCyc + (increment * i));
                		printf("\nDuty Cycle: %i", (startCyc + (increment * i)));
                		usleep(20000);
                	}
                	break;

           	case 1:  //Bicep
           			for (int i = 0; i < numPeriods; ++i) {
                		//REG_WRITE(gServos.test_base, Bicep_OFFSET, (startCyc + (increment * i)));
                		printf("\nDuty Cycle: %i", (startCyc + (increment * i)));
                		usleep(20000);
                	}
                	break;

          	case 2:  //Elbow
          			for (int i = 0; i < numPeriods; ++i) {
                		//REG_WRITE(gServos.test_base, Elbow_OFFSET, (startCyc + (increment * i)));
                		printf("\nDuty Cycle: %i", (startCyc + (increment * i)));
                		usleep(20000);
                	}
                	break;

           	case 3:  //Wrist
           			for (int i = 0; i < numPeriods; ++i) {
                		//REG_WRITE(gServos.test_base, Wrist_OFFSET, (startCyc + (increment * i)));
                		printf("\nDuty Cycle: %i", (startCyc + (increment * i)));
                		usleep(20000);
                	}
                	break;

           	case 4:  //Gripper
           			for (int i = 0; i < numPeriods; ++i) {
                		//REG_WRITE(gServos.test_base, Gripper_OFFSET, (startCyc + (increment * i)));
                		printf("\nDuty Cycle: %i", (startCyc + (increment * i)));
                		usleep(20000);
                	}
                	break;

           	default:
                	break;
	}
}


int main()
{
	servoMove(0, 10, 100, 36);

	return 0;
}
