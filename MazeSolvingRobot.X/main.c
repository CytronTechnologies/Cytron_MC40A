/*******************************************************
 *  Title: Maze Solving Robot
 *  Author: Idris Zainal Abidin
 *  PIC: PIC16F887
 *  Oscillator: 8 MHz (Internal)
 *  Software: MPLAB X IDE V2.10 + XC8 V1.31
 *  Last Modified: 4 Jan 2012
 *  Revision: 1.0
 *******************************************************/

/***** Include files *****/
#include "system.h"
#include "lcd.h"
#include "uart.h"
#include "pwm.h"

/***** PIC special fuction configuration *****/
#pragma config FOSC = INTRC_NOCLKOUT    // I/O function on RA6 & RA7
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = ON   // RE3/MCLR pin function is MCLR
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config LVP = OFF

/***** Function prototype *****/
void picInit(void);
void beep(uChar times, uInt delayMs);
void wifiString(const char *s);
void motor(sChar speedLM, sChar speedRM);

/***** Global variable *****/

/***** Main function *****/
void main(void)
{
  uInt i;
  uChar path[20], pathLength = 0, pathTotal, dir = 0;
  uChar simplified = 0, string[3];
  uLong buzzerCount = 0;

  picInit();
  lcdInit();
  beep(2, 50);

  lcdClear();
  lcdPutstr("SW1:Maze");
  lcdGoto(2,1);
  lcdPutstr("SW2:Line");

  while(1)
  {
    if(!SW1)
    {
      lcdClear();
      lcdPutstr("  Maze  ");
      lcdGoto(2,1);
      lcdPutstr("Solving.");
      delayMs(1000);
      lcdClear();
      lcdPutstr("Path:   ");
      beep(1, 50);
      break;
    }
    else if(!SW2)
    {
      lcdClear();
      lcdPutstr("Line    ");
      lcdGoto(2,1);
      lcdPutstr("Follower");
      beep(1, 50);
      while(1)
      {
        buzzerCount++;
        if(buzzerCount > 50000) buzzerCount = 0;
        else if(buzzerCount > 10000) BUZZER = 0;
        else if(buzzerCount > 5000) BUZZER = 1;
        else if(buzzerCount > 2000) BUZZER = 0;
        else if(buzzerCount > 1000) BUZZER = 1;
        
        if(!senLeft && !senMLeft && senMiddle && !senMRight && !senRight)
        {
          motor(80, 80);
        }
        else if(!senLeft && senMLeft && senMiddle && !senMRight && !senRight)
        {
          motor(40, 80);
        }
        else if(!senLeft && !senMLeft && senMiddle && senMRight && !senRight)
        {
          motor(80, 40);
        }
        else if(!senLeft && senMLeft && !senMiddle && !senMRight && !senRight)
        {
          motor(30, 80);
        }
        else if(!senLeft && !senMLeft && !senMiddle && senMRight && !senRight)
        {
          motor(80, 30);
        }
        else if(senLeft && senMLeft && !senMiddle && !senMRight && !senRight)
        {
          motor(10, 80);
        }
        else if(!senLeft && !senMLeft && !senMiddle && senMRight && senRight)
        {
          motor(80, 10);
        }
        else if(senLeft && !senMLeft && !senMiddle && !senMRight && !senRight)
        {
          motor(0, 80);
        }
        else if(!senLeft && !senMLeft && !senMiddle && !senMRight && senRight)
        {
          motor(80, 0);
        }
      }
    }
  }

  while(1)
  {
    if(!senMLeft && senMiddle && !senMRight)
    {
      motor(70, 70);
    }
    else if(senMLeft && senMiddle && !senMRight)
    {
      motor(40, 70);
    }
    else if(!senMLeft && senMiddle && senMRight)
    {
      motor(70, 40);
    }
    else if(senMLeft && !senMiddle && !senMRight)
    {
      motor(30, 70);
    }
    else if(!senMLeft && !senMiddle && senMRight)
    {
      motor(70, 30);
    }

    if(senRight && !senLeft)
    {
      motor(30, 30);
      for(i = 0; i < 200; i++)
      {
        if(senLeft) break;
        delayMs(1);
      }
      if(!senLeft)
      {
        motor(0, 0);
        delayMs(200);
        if(!senMiddle)
        {
          dir = 'R';
          motor(30, -30);
          while(!senRight);
//          motor(10, -10);
          while(!senMRight);
          motor(0, 0);
          delayMs(200);
        }
        else dir = 'S';
      }
    }
    if(senLeft)
    {
      motor(30, 30);
      delayMs(200);
      motor(0, 0);
      delayMs(200);
      dir = 'L';
      motor(-30, 30);
      while(!senLeft);
//      motor(-10, 10);
      while(!senMLeft);
      motor(0, 0);
      delayMs(200);
    }

    if(!senLeft && !senMLeft && !senMiddle && !senMRight && !senRight)
    {
      motor(0, 0);
      delayMs(200);
      dir = 'B';
      motor(30, -30);
      while(!senRight);
//      motor(10, -10);
      while(!senMRight);
      motor(0, 0);
      delayMs(200);
    }

    if(senLeft && senMLeft && senMiddle && senMRight && senRight)
    {
      motor(0, 0);
      beep(3, 300);
      pathTotal = pathLength;
      lcdGoto(1, 6);
      lcdNumber(pathTotal, DEC, 2);
      break;
    }

    if(dir)
    {
      if(dir == 'B')
      {
        simplified = 1;
        string[1] = dir; // 'B' always at the middle
        lcdGoto(2, pathLength+1);
        lcdPutchar(dir);
        string[0] = path[--pathLength];
        dir = 0;
      }
      else if(simplified)
      {
        string[2] = dir;
        /*LBL = S, LBR = B, LBS = R, RBL = B, SBL = R, SBS = B*/
        if(!memcmp("LBL", &string, 3)) path[pathLength] = 'S';
        else if(!memcmp("LBR", &string, 3)) path[pathLength] = 'B';
        else if(!memcmp("LBS", &string, 3)) path[pathLength] = 'R';
        else if(!memcmp("RBL", &string, 3)) path[pathLength] = 'B';
        else if(!memcmp("SBL", &string, 3)) path[pathLength] = 'R';
        else if(!memcmp("SBS", &string, 3)) path[pathLength] = 'B';

        lcdGoto(2, pathLength+1);
        lcdPutchar(path[pathLength]);
        lcdGoto(2, pathLength+2);
        lcdPutchar(' ');

        if(path[pathLength] == 'B')
        {
          string[1] = path[pathLength]; // 'B' always at the middle
          string[0] = path[--pathLength];
        }
        else
        {
          simplified = 0;
          pathLength++;
        }
        dir = 0;
      }
      else if(!simplified)
      {
        path[pathLength] = dir;
        lcdGoto(2, pathLength+1);
        lcdPutchar(path[pathLength]);
        pathLength++;
      }
      dir = 0;
    }
  }

  while(1)
  {
    if(!SW1)
    {
      pathLength = 0;
      beep(2, 50);
      delayMs(1000);
      while(1)
      {
        if(!senMLeft && senMiddle && !senMRight)
        {
          motor(70, 70);
        }
        else if(senMLeft && senMiddle && !senMRight)
        {
          motor(40, 70);
        }
        else if(!senMLeft && senMiddle && senMRight)
        {
          motor(70, 40);
        }
        else if(senMLeft && !senMiddle && !senMRight)
        {
          motor(30, 70);
        }
        else if(!senMLeft && !senMiddle && senMRight)
        {
          motor(70, 30);
        }

        if(pathLength < pathTotal)
        {
          if(senLeft || senRight)
          {
            if(path[pathLength] == 'L')
            {
              motor(30, 30);
              delayMs(200);
              motor(0, 0);
              delayMs(200);
              motor(-30, 30);
              while(!senLeft);
  //              motor(-10, 10);
              while(!senMLeft);
              motor(0, 0);
              delayMs(200);
            }
            else if(path[pathLength] == 'R')
            {
              motor(30, 30);
              delayMs(200);
              motor(0, 0);
              delayMs(200);
              motor(30, -30);
              while(!senRight);
  //              motor(-10, 10);
              while(!senMRight);
              motor(0, 0);
              delayMs(200);
            }
            else if(path[pathLength] == 'S')
            {
              motor(30, 30);
              while(senLeft || senRight);
            }
            pathLength++;
          }
        }

        if(senLeft && senMLeft && senMiddle && senMRight && senRight)
        {
          motor(0, 0);
          beep(10, 50);
          break;
        }
      }
    }
  }
}

/***** Sub function *****/
void picInit(void)
{
  IRCF2 = 1; // IRCF<2:0> = 111 => 8MHz
  IRCF1 = 1; // IRCF<2:0> = 110 => 4MHz, default
  IRCF0 = 1; // IRCF<2:0> = 101 => 2MHz, PORTA = 0;

  ANSEL = 0; // Set all AN pins as digital
  ANSELH = 0;

  PORTA = 0; // Clear all pins at PORTA
  PORTB = 0; // Clear all pins at PORTB
  PORTC = 0; // Clear all pins at PORTC
  PORTD = 0; // Clear all pins at PORTD
  PORTE = 0; // Clear all pins at PORTE

  TRISA = 0b00111000; // Set TRISA, 0:output, 1:input
  TRISB = 0b00000011; // Set TRISB, 0:output, 1:input
  TRISC = 0b10000000; // Set TRISC, 0:output, 1:input
  TRISD = 0b00000000; // Set TRISD, 0:output, 1:input
  TRISE = 0b011; // Set TRISE, 0:output, 1:input
}

void beep(uChar times, uInt delayMs)
{
  uInt loop;
  for(; times > 0; times--)
  {
    BUZZER = 1;
    for(loop = 0; loop < delayMs; loop++) delayMs(1);
    BUZZER = 0;
    for(loop = 0; loop < delayMs; loop++) delayMs(1);
  }
}

void motor(sChar speedLM, sChar speedRM)
{
  uChar speed, maxSpeed = 80;

  if(speedLM < 0) // if speedLM is (-) value
  {
    motorLeftA = 1; // Left motor rotate backward
    motorLeftB = 0;
    speed = (-1) * speedLM; // Change the (-) to (+) value
  }
  else if(speedLM > 0) // if speedLM is (+) value
  {
    motorLeftA = 0; // Left motor rotate forward
    motorLeftB = 1;
    speed = speedLM;
  }
  else if(!speedLM) // if speedLM is null
  {
    motorLeftA = 0; // Left motor stop
    motorLeftB = 0;
    speed = speedLM;
  }
  if(speed > maxSpeed) speed = maxSpeed; // Limit the speed
  setPwmRC1(1000, speed); // Set freq = 1000, duty cycle = speed

  if(speedRM < 0) // if speedRM is (-) value
  {
    motorRightA = 1; // Right motor rotate backward
    motorRightB = 0;
    speed = (-1) * speedRM; // Change the (-) to (+) value
  }
  else if(speedRM > 0) // if speedRM is (+) value
  {
    motorRightA = 0; // Right motor rotate forward
    motorRightB = 1;
    speed = speedRM;
  }
  else if(!speedRM) // if speedRM is numm
  {
    motorRightA = 0; // Right motor stop
    motorRightB = 0;
    speed = speedRM;
  }
  if(speed > maxSpeed) speed = maxSpeed; // Limit the speed
  setPwmRC2(1000, speed); // Set freq = 1000, duty cycle = speed
}

