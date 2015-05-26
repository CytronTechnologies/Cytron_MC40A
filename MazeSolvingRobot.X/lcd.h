#ifndef LCD_H
#define	LCD_H

/***********************************
 * lcdInit();
 * lcdClear();
 * lcdHome();
 * lcdGoto(1,1);
 * lcdPutchar('A');
 * lcdPutstr("Hello World");
 * lcdNumber(123,DEC,3);
 ***********************************/

/***** Include files *****/
#include "system.h"

/***** Define *****/
#define lcdPulse()      ((LCD_E=1), delayUs(2), (LCD_E=0), delayUs(2))
#define lcdConfig(x)    lcdWrite(0, x)
#define lcdPutchar(x)   lcdWrite(1, x)
#define lcdClear()      (lcdConfig(1), delayMs(2))
#define lcdHome()       (lcdConfig(2), delayMs(2))

/***** LCD function prototype *****/
void lcdInit(void);
void lcdWrite(char rs, uChar data);
void lcdGoto(uChar row, uChar col);
void lcdPutstr(const char *s);
void lcdNumber(uInt no, uChar base, uChar digit);

/***** Global variable *****/

/***** LCD sub function *****/
void lcdInit(void)
{
  delayMs(20);
  lcdConfig(0x30); // 8-bits function set
  lcdConfig(0x30); // 8-bits function set
  lcdConfig(0x30); // 8-bits function set
  lcdConfig(0x38); // 8-bits function set
  lcdConfig(0x0C); // Display ON/OFF control
  lcdConfig(0x01); // Clear screen
  lcdConfig(0x06); // Entry mode set
  lcdConfig(0x02); // Return to home
  delayMs(2);
}

void lcdWrite(uChar rs, uChar data)
{
  LCD_RS = rs;
  LCD_DATA = data;
  lcdPulse();
  delayUs(40);
}

void lcdGoto(uChar row, uChar col)
{
  switch(row)
  {
  case 1: lcdConfig(0x80 + col - 1);
    break;
  case 2: lcdConfig(0xC0 + col - 1);
    break;
  case 3: lcdConfig(0x94 + col - 1);
    break;
  case 4: lcdConfig(0xD4 + col - 1);
    break;
  }
}

void lcdPutstr(const char *s)
{
  while(*s >= ' ' && *s <= '~') lcdPutchar(*s++);
}

void lcdNumber(uInt no, uChar base, uChar digit)
{
  uChar i, j, di[10];
  for(i = 0; i < 10; i++) di[i] = 0;
  i = 0;

  do
  {
    di[i++] = no % base;
    no = no / base;
  }
  while(no);

  for(j = digit; j > 0; j--)
  {
    if(di[j - 1] < 10) lcdPutchar(di[j - 1] + '0');
    else lcdPutchar(di[j - 1] - 10 + 'A');
  }
}

#endif
