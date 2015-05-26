#ifndef PWM_H
#define	PWM_H

/***** Include files *****/
#include "system.h"

/***** PWM Function Prototype *****/
void setPwmRC1(int freq, int duty);
void setPwmRC2(int freq, int duty);

/***** PWM Sub Function *****/
void setPwmRC1(int freq, int duty)
{
  int a;
  T2CON = 0b00000111;
  CCP2CON = 0b00001100;
  PR2 = (_XTAL_FREQ / (4 * 16)) / (freq) - 1;
  a = (PR2 + 1) * duty / 25;
  CCPR2L = a / 4;
  CCP2CON = (CCP2CON & 0b11001111) | ((a % 4) << 4);
}

void setPwmRC2(int freq, int duty)
{
  int a;
  T2CON = 0b00000111;
  CCP1CON = 0b00001100;
  PR2 = (_XTAL_FREQ / (4 * 16)) / (freq) - 1;
  a = (PR2 + 1) * duty / 25;
  CCPR1L = a / 4;
  CCP1CON = (CCP1CON & 0b11001111) | ((a % 4) << 4);
}

#endif