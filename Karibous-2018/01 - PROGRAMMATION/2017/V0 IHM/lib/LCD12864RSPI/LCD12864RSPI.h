
//Arduino library v1.0
//Demo LCD12864 spi
//www.dfrobot.com

#ifndef LCD12864RSPI_h
#define LCD12864RSPI_h
#include <avr/pgmspace.h>
#include <inttypes.h>


class LCD12864RSPI {
	typedef unsigned char uchar;

public:

	LCD12864RSPI();

	void initDriverPin(int clock,int latch,int data);
	void initialise(void);         /* Initialize the screen. */
	void delayns(void);            /* Delay dalayTime amount of microseconds. */

	void writeByte(int data);      /* Write byte data. */
	void writeCommand(int cmd);    /* Write command cmd into the screen. */
	void writeData(int data);      /* Write data data into the screen. */
	void moveCursor(int X, int Y); /* Move cursor to 0-based column X and 0-based row Y as the starting point of the next display. */

	void clear(void);			                    /* Clear the screen. */
	void clearRow(int Y);                           /* Clear the y-th row (y is 0-based) */
	void fillBlock(int X, int Y, uchar c, int len); /* Fill block starting from 0-based column X and 0-based row Y with the character c. The filled block is of length len.*/
	void clearBlock(int X, int Y, int length);      /* Clear the block starting from 0-based column X and 0-based row Y. The cleared block is of len byte long. */

	void printf(int X, int Y, char *fmt, ... );         /* Display formated string at 0-based column X and 0-based row Y. The formatted string should not be longer than 64 characters. */
	void displayString(int X,int Y,uchar *ptr,int dat); /* Display string ptr (with length len) at 0-based column X and 0-based row Y. */
	void displaySig(int M,int N,int sig);               /* Display a single character sig at 0-based column X and 0-based row Y. */
	void displayInteger(int X, int Y, int number);      /* Display integer number at 0-based column X and 0-based row Y. */

	void drawFullScreen(uchar *p);  /* Display the full screen using data from p. */
	void drawImage1(uchar img[]);   /* Display image */
	void drawImage2(uchar img[]);   /* Display image */


	int delayTime;
	int DEFAULTTIME;


	//static const int latchPin = 8;
	//static const int clockPin = 3;
	//static const int dataPin = 9;

	int latchPin ;
	int clockPin ;
	int dataPin ;
};
extern LCD12864RSPI LCDA;
#endif
