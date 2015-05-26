#ifndef UART_H
#define	UART_H

/***** Include Files *****/
#include "system.h"

/***** UART Function Prototype *****/
void uartInit(uLong baudRate);
void uartTransmit(uChar dataTx);
void uartPutstr(const char *s);
uChar uartReceive(void);
void uartNumber(uInt no, uChar base, uChar digit);

/***** Global Variable *****/

/***** Interrupt Function *****/

/***** UART Sub Function *****/
void uartInit(uLong baudrate)
{
  TXEN = 1; // Enable transmission
  TX9 = 0; // 8-bit transmission
  RX9 = 0; // 8-bit reception
  CREN = 1; // Enable reception
  SPEN = 1; // Enable serial port

  if(baudrate > 9000)
  {
    BRGH = 1; // Baudrate high speed option
    SPBRG = (uInt) (((float) _XTAL_FREQ / (float) baudrate / 16.0) - 0.5);
  }
  else if(baudrate < 9000)
  {
    BRGH = 0; // Baudrate low speed option
    SPBRG = (uInt) (((float) _XTAL_FREQ / (float) baudrate / 64.0) - 0.5);
  }
}

void uartTransmit(uChar dataTx)
{
  while(!TXIF);
  TXREG = dataTx;
}

void uartPutstr(const char *s)
{
  while(*s) uartTransmit(*s++);
}

uChar uartReceive(void)
{
  if(OERR)
  {
    CREN = 0;
    CREN = 1;
  }
  while(!RCIF);
  return RCREG;
}

void uartNumber(uInt no, uChar base, uChar digit)
{
  char i, j, di[10];
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
    if(di[j - 1] < 10) uartTransmit(di[j - 1] + '0');
    else uartTransmit(di[j - 1] - 10 + 'A');
  }
}

#endif