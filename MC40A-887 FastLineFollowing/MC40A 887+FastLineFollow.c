/*******************************************************************************
* This is the main program to sanity test the MC40A MiniBot Controller using PIC16F887
*
* Author: Ober Choo Sui Hong @ Cytron Technologies Sdn. Bhd.
* This sample code is modified from MC40A 887 Template, for fast line following
* it require SPG10-30K + 46mmx10mm mini wheel and LSS05, lipo 7.4V 2 cells battery, of course other component too
* sample code is provided free, Cytron do not hold resposibilities to improve or modify
* User are free to modify and share it
* Date: 14 Jan 11
*******************************************************************************/
#include <htc.h>

/*******************************************************************************
* DEVICE CONFIGURATION WORDS FOR PIC16F887                                     *
*******************************************************************************/

// If PIC16F887 is being selected as device
__CONFIG(INTIO &		// Internal Clock, pins as I/O.
		 //HS &			// External Crystal at High Speed
		 WDTDIS &		// Disable Watchdog Timer.
		 PWRTEN &		// Enable Power Up Timer.
		 BORDIS &		// Disable Brown Out Reset.
		 MCLREN &		// MCLR function is enabled
		 LVPDIS);		// Disable Low Voltage Programming.


// ================================= Define section ===========================================
// Oscillator Frequency.
#define	_XTAL_FREQ		8000000		//using internal osc

// UART baud rate
#define UART_BAUD		9600

// I/O Connections.
// Parallel 2x16 Character LCD
#define LCD_E			RE2		// E clock pin is connected to RB5	
#define LCD_RS			RB6		// RS pin is used for LCD to differentiate data is command or character
#define	LCD_DATA		PORTD	// Data port of LCD is connected to PORTD, 4 bit mode								

// LED on MC40A
#define LED1			RB7

// Push button on MC40A
#define SW1				RB0		
#define SW2				RB1

// Digital limit switch
#define LIMIT1			RA1		//limit switch 1 
#define LIMIT2			RA2		//limit switch 2

// Buzzer
#define BUZZER			RB7

// L293B, H-Bridge IC to drive either DC brush or Stepper Motor
#define ML_EN			RC1		// this pin is connected to Enable of L293 H-bridge driver, it is being use for speed control, for Motor left
#define MR_EN			RC2		// this pin is connected to Enable of L293 H-bridge driver, it is being use for speed control, for Motor right

// L293 pin for DC Brushed Motor
#define MR_1			RB2		//right motor pin 1
#define MR_2			RB3		//right motor pin 2
#define ML_1			RB4		//left motor pin 1
#define ML_2			RB5		//left motor pin 2

// LSS05 sensor input assignment
#define SEN1			RA3		
#define SEN2			RA4
#define SEN3			RA5
#define SEN4			RE0
#define SEN5			RE1

#define LEFT			RA3
#define M_LEFT			RA4
#define MIDDLE			RA5
#define M_RIGHT			RE0
#define RIGHT			RE1

#define LSS_CAL			RC5		// pin to press Cal/mode button of LSS05


/*******************************************************************************
* PRIVATE FUNCTION PROTOTYPES                                                  *
*******************************************************************************/
//Line Following functions
void fast_line_follow(void);	
void calibrate_LSS05(void);
// ADC functions
void adc_init(void);
unsigned int ui_adc_read(void);
// PWM functions
void pwm_init(void);
void set_pwmr(unsigned char uc_duty_cycle);
void set_pwml(unsigned char uc_duty_cycle);
// LCD functions
void send_lcd_data(unsigned char b_rs, unsigned char uc_data);
void set_lcd_e(unsigned char b_output);
void set_lcd_rs(unsigned char b_output);
void set_lcd_data(unsigned char uc_data);
void lcd_init(void);
void lcd_clr(void);
void lcd_home(void);
void lcd_2ndline(void);
void lcd_goto(unsigned char uc_position);
void lcd_putchar(char c_data);
void lcd_putstr(const char* csz_string);
// SKPS functions
unsigned char uc_skps(unsigned char uc_data);
void skps_vibrate(unsigned char uc_motor, unsigned char uc_value);
void skps_reset(void);

void delay_ms(unsigned int ui_value);
void beep(unsigned char uc_count);
void SKPS_control(void);
// functions for line following
void motor(unsigned char uc_left_motor_speed,unsigned char uc_right_motor_speed);


/*******************************************************************************
* Global Variables                                                             *
*******************************************************************************/


/*******************************************************************************
* MAIN FUNCTION                                                                *
*******************************************************************************/
int main(void)
{
	unsigned char test_no = 1;
	
	// Initialize the Internal Osc, under OSCCON register
	IRCF2 = 1;		// IRCF<2:0> = 111 => 8MHz
	IRCF1 = 1;		// IRCF<2:0> = 110 => 4MHz, default
	IRCF0 = 1;		// IRCF<2:0> = 101 => 2MHz, PORTA = 0;
	
	// clear port value
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;
	
	// Initialize the I/O port direction.
	TRISA = 0b11111111;
	TRISB = 0b00000011;
	TRISC = 0b10011000;
	TRISD = 0;
	TRISE = 0b00000011;	
	
	// Initialize ADC.
	adc_init();
	
	// Initialize PWM.
	pwm_init();
	
	// Initialize the LCD.
	lcd_init();		// call this function is 2x8 LCD is connected to MC40A
			
	// Display the messages and beep twice.
	lcd_clr();
	lcd_putstr(" Cytron \n  Tech  ");
	beep(2);
	delay_ms(2000);
		
	lcd_clr();
	lcd_putstr("  SW1\nto start");
	while(SW1 == 1) continue;		// wait for SW1 to be pressed
	calibrate_LSS05();		
	lcd_clr();
	lcd_putstr("Cal Done\nLine Fol");
	delay_ms(1500);			//wait for 1.5 second
	fast_line_follow();		
}



/*******************************************************************************
* PRIVATE FUNCTION: delay_ms
*
* PARAMETERS:
* ~ ui_value	- The period for the delay in miliseconds.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Delay in miliseconds.
*
*******************************************************************************/
void delay_ms(unsigned int ui_value)
{
	while (ui_value-- > 0) {
		__delay_ms(1);
	}	
}	



/*******************************************************************************
* PRIVATE FUNCTION: beep
*
* PARAMETERS:
* ~ uc_count	- How many times we want to beep.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Beep for the specified number of times.
*
*******************************************************************************/
void beep(unsigned char uc_count)
{
	while (uc_count-- > 0) {
		BUZZER = 1;
		delay_ms(50);
		BUZZER = 0;
		delay_ms(50);
	}
}
// ============================ line following functions ================================
/*******************************************************************************
* PUBLIC FUNCTION: fast_line_follow
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* perform line following in fast speed. This function must use SPG10-30K and 46x10mm mini wheel,
* the position of motor to LSS05 is very important.
*******************************************************************************/
void fast_line_follow(void)
{
	lcd_clr();
	lcd_putstr("  MC40A\nLine Fol");
	//motor right forward	
	MR_1 = 1;
	MR_2 = 0;
	//motor left forward
	ML_1 = 0;
	ML_2 = 1;
	while(1)
	{
	if((M_LEFT == 0)&&(MIDDLE == 1)&&(M_RIGHT == 0)) //check middle, middle left and middle right sensor
														//assuming black line, dark ON
		{		
			motor(85,85);	// robot move straight with both left and right motor moving at same speed
			//motor(uc_left_motor_speed, uc_right_motor_speed)			
		}
		
		else if((M_LEFT == 1)&&(MIDDLE == 1)&&(M_RIGHT == 0)) // robot has move to left a little bit
		{			
			motor(80,85);	// robot turning to left
			//motor(120,200);
		}		
		
		else if((M_LEFT == 0)&&(MIDDLE == 1)&&(M_RIGHT == 1)) // robot has move to right a little bit
		{			
			motor(85,80);	// robot turning to right
			//motor(200,120);
		}
		
		else if((LEFT == 0)&&(M_LEFT == 1)&&(MIDDLE == 0))	// robot has move to left
		{			
			motor(65,83);	// robot turning to Left, hard
			//motor(80,180);
		}
				
		else if((MIDDLE == 0)&&(M_RIGHT == 1)&&(RIGHT == 0))// robot has move to right
		{		
			motor(83,65);	// robot turning to right, hard
			//motor(180,80);
		}	
		else if((LEFT == 1)&&(M_LEFT == 1))	// robot has move to left
		{			
			motor(20,80);	// robot turning to Left, hard
			//motor(55,140);
		}
				
		else if((M_RIGHT == 1)&&(RIGHT == 1))// robot has move to right
		{		
			motor(80,20);	// robot turning to right, hard
			//motor(140,55);
		}	
		else if((LEFT == 1)&&(M_LEFT == 0))	// robot has move to the most left side
		{			
			motor(0,80);	// robot turning to Left, hard
			//motor(0,150);
		}
				
		else if((M_RIGHT == 0)&&(RIGHT == 1))// robot has move to the most right site
		{		
			motor(80,0);	// robot turning to right, hard
			//motor(150,0);
		}	
	}//while(1)
	
}	
/*******************************************************************************
* PUBLIC FUNCTION: calibrate_LSS05
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Autocalibrate LSS05, but require the motor to be SPG10-30K and 46x10mm mini wheel.
* Using Lipo, 2 cells, 850mAh
*******************************************************************************/
void calibrate_LSS05(void)
{
	LSS_CAL = 1; 	// using the transistor on board to pull low the LSS05 calibration switch
	delay_ms(10);	// delay for short delay of time
	LSS_CAL = 0;	// release the low signal on LSS05 calibration switch
	delay_ms(1000);	// wait for LSS05 to start calibration
	//calibration will start, pivot right and keep round	
	//motor right reverse	
	MR_1 = 0;
	MR_2 = 1;
	//motor left forward
	ML_1 = 0;
	ML_2 = 1;
	motor(75, 75);	// pivot right with low speed for LSS05 to detect line
	delay_ms(100);
	motor(57, 57);	// pivot right with low speed for LSS05 to detect line
	delay_ms(7000);	// wait for 6 second
	
	while(SEN5 == 0) continue; //wait for sensor right to detect line, when detect line is high for dark on 
	motor(53, 53);				// change to lower speed while approaching center			
	while(SEN3 == 0) continue; //wait for sensor middle to detect line, when detect line is high for dark on 
	delay_ms(10);
	//motor right brake	
	MR_1 = 0;
	MR_2 = 0;
	//motor left brake
	ML_1 = 0;
	ML_2 = 0;		
}

// ================================== ADC functions ======================================

/*******************************************************************************
* PUBLIC FUNCTION: adc_init
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Initialize the ADC module.
*
*******************************************************************************/
void adc_init(void)
{
	// A/D Conversion Clock = FOSC/32.
	ADCS1 = 1;
	ADCS0 = 0;
	
	// Set AN0 as analog input only, the rest is digital I/O.
	ANS0 = 1;	// AN0 is analog input
	ANS1 = 0;	// AN1 is digital I/O
	ANS2 = 0;	// AN2 is digital I/O
	ANS3 = 0;	// AN3 is digital I/O
	ANS4 = 0;	// AN4 is digital I/O
	ANS5 = 0;	// AN5 is digital I/O
	ANS6 = 0;	// AN6 is digital I/O
	ANS7 = 0;	// AN7 is digital I/O
	ANS8 = 0;	// AN8 is digital I/O
	ANS9 = 0;	// AN9 is digital I/O
	ANS10 = 0;	// AN10 is digital I/O
	ANS11 = 0;	// AN11 is digital I/O
	ANS12 = 0;	// AN12 is digital I/O
	ANS13 = 0;	// AN13 is digital I/O
	// Configure Vref (-) to Vss
	VCFG1 = 0;

	// Configure Vref (+) to Vdd
	VCFG0 = 0;

	// Configure the result to be right justified.
	ADFM = 1;
	
	// Turn OFF ADC by default.
	ADON = 0;
}	



/*******************************************************************************
* PUBLIC FUNCTION: ui_adc_read
*
* PARAMETERS:
* void
*
* RETURN:
* ~ The ADC result in 16 bit
*
* DESCRIPTIONS:
* Convert and read the result of the ADC at channel 0.
*
*******************************************************************************/
extern unsigned int ui_adc_read(void)
{
	unsigned int temp = 0;
	// Select the ADC channel.
	CHS3 = 0;	//select channel 0
	CHS2 = 0;
	CHS1 = 0;
	CHS0 = 0;
	
	// Delay 5mS to fully charge the holding capacitor in the ADC module.
	__delay_ms(5);
	
	// Start the conversion and wait for it to complete.
	GODONE = 1;
	while (GODONE == 1);
	
	// Return the ADC result.
	temp = ADRESH << 8;
	temp = temp + ADRESL;
	return temp;
}	
// ================================== PWM functions ======================================
/*******************************************************************************
* PUBLIC FUNCTION: pwm_init
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Initialize the CCP1 and CCP2 module to operate in PWM mode.
*
*******************************************************************************/
void pwm_init(void)
{
	// Setting PWM frequency = 4.90KHz at 8MHz OSC Freq
	PR2 = 0x65;
	T2CKPS1 = 0;
	T2CKPS0 = 1;	// Timer 2 prescale = 4.
	
	CCPR1L = 0;		// Duty cycle = 0;
	TMR2ON = 1;		// Turn on Timer 2.	
	
	//configuration for CCP1CON
	P1M1 = 0;		//CCP1, P1A as single output
	P1M0 = 0;

	DC1B1 = 0;		// 2 LSB of 10 PWM, make it 00
	DC1B0 = 0;
	
	CCP1M3 = 1;		// Configure CCP1 module to operate in PWM mode.
	CCP1M2 = 1;		
	CCP1M1 = 0;
	CCP1M0 = 0;	
	
	//configuration for CCP2CON
	CCP2X = 0;		// 2 LSB of 10 PWM, make it 00
	CCP2Y = 0;

	CCP2M3 = 1;		// Configure CCP1 module to operate in PWM mode.
	CCP2M2 = 1;		
	CCP2M1 = 0;
	CCP2M0 = 0;	
}	



/*******************************************************************************
* PUBLIC FUNCTION: set_pwm1
*
* PARAMETERS:
* ~ uc_duty_cycle	- The duty cycle of the PWM1.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Set the duty cycle of the PWM1.
*
*******************************************************************************/
void set_pwmr(unsigned char uc_duty_cycle)
{
	CCPR1L = uc_duty_cycle;
}	

/*******************************************************************************
* PUBLIC FUNCTION: set_pwm2
*
* PARAMETERS:
* ~ uc_duty_cycle	- The duty cycle of the PWM2.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Set the duty cycle of the PWM2.
*
*******************************************************************************/
void set_pwml(unsigned char uc_duty_cycle)
{
	CCPR2L = uc_duty_cycle;
}	

// ================================== LCD functions ======================================

/*******************************************************************************
* PUBLIC FUNCTION: lcd_init
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Initialize and clear the LCD display.
*
*******************************************************************************/
void lcd_init(void)
{
	// Set the LCD E pin and wait for the LCD to be ready before we
	// start sending data to it.
	set_lcd_e(1);
	__delay_ms(15);
	
	// Configure the Function Set of the LCD.	
	// Because of the LCD is initialized as 8-bit mode during start up, we need
	// to send the data in 8-bit mode to configure the LCD.
	send_lcd_data(0, 0b00111000);	
	
	// Configure the entry mode set of the LCD.
	send_lcd_data(0, 0b00000110);
	
	// Configure the display on/off control of the LCD.
	send_lcd_data(0, 0b00001100);
	
	// Clear the LCD display.
	lcd_clr();
}



/*******************************************************************************
* PUBLIC FUNCTION: lcd_clr
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Clear the LCD display and return the cursor to the home position.
*
*******************************************************************************/
void lcd_clr(void)
{
	// Send the command to clear the LCD display.
	send_lcd_data(0, 0b00000001);
}



/*******************************************************************************
* PUBLIC FUNCTION: lcd_home
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Return the cursor to the home position.
*
*******************************************************************************/
void lcd_home(void)
{
	// Send the command to return the cursor to the home position.
	send_lcd_data(0, 0b00000010);
}



/*******************************************************************************
* PUBLIC FUNCTION: lcd_2ndline
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Jump to the second line of the LCD display.
*
*******************************************************************************/
void lcd_2ndline(void)
{
	// Send the command to jump to the second row.
	send_lcd_data(0, 0b11000000);
}



/*******************************************************************************
* PUBLIC FUNCTION: lcd_goto
*
* PARAMETERS:
* ~ uc_position	- The position that we want to set the cursor at.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Jump to the defined position of the LCD display.
*
*******************************************************************************/
void lcd_goto(unsigned char uc_position)
{
	// Send the command to jump to the defined position.
	send_lcd_data(0, 0b10000000 | uc_position);
}



/*******************************************************************************
* PUBLIC FUNCTION: lcd_putchar
*
* PARAMETERS:
* ~ c_data	- The character to display.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Display a character on the LCD.
*
*******************************************************************************/
void lcd_putchar(char c_data)
{
	// Send the data to display.
	send_lcd_data(1, (unsigned char)c_data);
}



/*******************************************************************************
* PUBLIC FUNCTION: lcd_putstr
*
* PARAMETERS:
* ~ csz_string	- The null terminated string to display.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Display a string on the LCD.
*
*******************************************************************************/
void lcd_putstr(const char* csz_string)
{
	// Loop until the end of string.
	while (*csz_string != '\0') {
		
		// Jump to the second row if '\n' or '\r' is found.
		if (*csz_string == '\n' || *csz_string == '\r') {
			lcd_2ndline();
		}
		
		// Else, display the character.
		else {
			lcd_putchar(*csz_string);
		}
		
		// Point to next character.
		csz_string++;
	}
}



/*******************************************************************************
* PRIVATE FUNCTION: send_lcd_data
*
* PARAMETERS:
* ~ b_rs		- The output of the LCD RS pin (1 or 0).
* ~ uc_data		- The output of the LCD data bus.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Set the output of the LCD RS pin and data bus.
*
*******************************************************************************/
void send_lcd_data(unsigned char b_rs, unsigned char uc_data)
{
		// 8-bit Mode.
		// We only need to send the data once.
		set_lcd_rs(b_rs);
		set_lcd_data(uc_data);
		
		// Send a negative e pulse.
		set_lcd_e(0);
		__delay_ms(2);
		set_lcd_e(1);
		__delay_ms(1);		
}



/*******************************************************************************
* PRIVATE FUNCTION: set_lcd_e
*
* PARAMETERS:
* ~ b_output	- The output of the LCD E pin (1 or 0).
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Set the output of the LCD E pin.
*
*******************************************************************************/
void set_lcd_e(unsigned char b_output)
{
	LCD_E = b_output;
}



/*******************************************************************************
* PRIVATE FUNCTION: set_lcd_rs
*
* PARAMETERS:
* ~ b_output	- The output of the LCD RS pin (1 or 0).
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Set the output of the LCD RS pin.
*
*******************************************************************************/
void set_lcd_rs(unsigned char b_output)
{
	LCD_RS = b_output;
}



/*******************************************************************************
* PRIVATE FUNCTION: set_lcd_data
*
* PARAMETERS:
* ~ uc_data	- The output of the LCD data bus.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Set the output of the LCD data bus.
*
*******************************************************************************/
void set_lcd_data(unsigned char uc_data)
{
	LCD_DATA = uc_data;
}



/*******************************************************************************
* PRIVATE FUNCTION: motor
*
* PARAMETERS:
* ~ uc_left_motor_speed
* ~ uc_right_motor_speed
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* move motor forward and change speed.
*
*******************************************************************************/
void motor(unsigned char uc_left_motor_speed,unsigned char uc_right_motor_speed)
{	
	//set the speed for left and right motor
	set_pwmr(uc_right_motor_speed);
	set_pwml(uc_left_motor_speed);	
}







