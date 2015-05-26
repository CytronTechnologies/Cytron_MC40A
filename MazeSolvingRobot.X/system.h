#ifndef SYSTEM_H
#define	SYSTEM_H

/***** Include files *****/
#include <htc.h>
#include <string.h>

/***** Define *****/
#define	_XTAL_FREQ  8000000

#define SW1 RB0
#define SW2 RB1

#define BUZZER  RB7
#define UARTRST RC0

#define motorRightA RB2
#define motorRightB RB3
#define motorLeftA  RB4
#define motorLeftB  RB5

#define senLeft   RA3
#define senMLeft  RA4
#define senMiddle RA5
#define senMRight RE0
#define senRight  RE1
#define senCal    RC5

#define LCD_RS   RB6
#define LCD_E    RE2
#define LCD_DATA PORTD

#define HEX     16
#define DEC     10
#define OCT     8
#define BIN     2

#define delayMs(x) __delay_ms(x)
#define delayUs(x) __delay_us(x)

typedef unsigned char uChar;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef signed char sChar;
typedef signed int sInt;
typedef signed long sLong;

#endif