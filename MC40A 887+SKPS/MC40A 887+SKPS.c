/*******************************************************************************
* This is the main program to sanity test the MC40A MiniBot Controller using PIC16F887
*
* Author: Ober Choo Sui Hong @ Cytron Technologies Sdn. Bhd.
* This sample code is modified from MC40A 887 Template, for SKPS control
* it require SKPS and wired ro wireless PS2 controller
* sample code is provided free, Cytron do not hold resposibilities to improve or modify
* User are free to modify and share it
*******************************************************************************/
#include <stdio.h>
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

// SK's Reset
#define SK_R			RC0		// pin to reset SK
//SKPS command constants
#define	p_select		0
#define p_joyl			1
#define p_joyr			2
#define p_start			3
#define p_up			4
#define p_right			5
#define p_down			6
#define p_left			7
#define	p_l2			8
#define	p_r2			9
#define p_l1			10
#define p_r1			11
#define p_triangle		12
#define p_circle		13
#define p_cross			14
#define	p_square		15
#define p_joy_lx		16
#define	p_joy_ly		17
#define p_joy_rx		18
#define p_joy_ry		19
#define p_joy_lu		20
#define p_joy_ld		21
#define p_joy_ll		22
#define p_joy_lr		23
#define p_joy_ru		24
#define p_joy_rd		25
#define p_joy_rl		26
#define p_joy_rr		27

#define	p_con_status	28
#define p_motor1		29
#define p_motor2		30


/*******************************************************************************
* PRIVATE FUNCTION PROTOTYPES                                                  *
*******************************************************************************/
// UART functions
void uart_init(void);
void uart_tx(unsigned char uc_data);
unsigned char uc_uart_rx(void);
void uart_putstr(const char* csz_string);
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

char string_buffer[40] = {0};
char string_SWsError[] = "Other \nSWs Low";
char string_passed[] = "Passed!";


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
	TRISC = 0b10111000;
	TRISD = 0;
	TRISE = 0b00000011;	
	
	// Initialize UART.
	uart_init();
	
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
	
			
	while (1) 
	{
		SKPS_control();	
	} // while (1)	
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

/*******************************************************************************
* PRIVATE FUNCTION: test_SKPS
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the SKPS.
*
*******************************************************************************/

void SKPS_control(void)
{
	unsigned char uc_skps_ly = 0 , uc_skps_lx = 0, uc_skps_ry = 0;
	unsigned char max_speed = 40;
	// Display the messages.
	lcd_clr();
	lcd_putstr("  Demo\n  SKPS");
	delay_ms(1000);
	
	// Display the messages.
	skps_reset();	//reset the SKPS
	delay_ms(500);
	lcd_clr();
	lcd_putstr("Connect\n  PS2");
	while (uc_skps(p_con_status) == 0) continue;
	lcd_clr();
	lcd_putstr("   PS2\nDetected");
	delay_ms(2000);
	lcd_clr();
	lcd_putstr(" Press\n START");	
	
	while(uc_skps(p_start)==1) continue; //wait for X on PS2 to be press	
	lcd_clr();
	lcd_putstr("  PS2\nConnect!");
	
	
	while (uc_skps(p_cross)==1) 
	{	
		if(!(uc_skps(p_l1) && uc_skps(p_l2) && uc_skps(p_r1) && uc_skps(p_r2)))
		{			
			BUZZER = 1;
		}	
		else 
		{
			BUZZER = 0;		
		}
		
		uc_skps_ry = uc_skps(p_joy_ry);		// read the value of right joystik, y axis
		uc_skps_lx = uc_skps(p_joy_lx);		// read the value of left joystick, x axis
		uc_skps_ly = uc_skps(p_joy_ly);		// read the value of left joystick, y axis
		
		if(uc_skps_ly < 100)  //left joystick being push up
		{
			if(uc_skps_lx < 100) // left joystick being push left
			{
			ML_1 = 0;	// left motor stop
			ML_2 = 0;
			MR_1 = 1;	// right motor forward
			MR_2 = 0;
			}
			else if (uc_skps_lx > 156)// left joystick being push right
			{
			ML_1 = 0;	// left motor forward
			ML_2 = 1;
			MR_1 = 0;	// right motor stop
			MR_2 = 0;
			}
			else
			{
			ML_1 = 0;	// left motor forward
			ML_2 = 1;
			MR_1 = 1;	// right motor forward
			MR_2 = 0;
			}			
			
		}
		else if(uc_skps_ly > 156) //left joystick being push down
		{
			if(uc_skps_lx < 100) // left joystick being push left
			{
			ML_1 = 0;	// left motor stop
			ML_2 = 0;
			MR_1 = 0;	// right motor reverse
			MR_2 = 1;
			}
			else if (uc_skps_lx > 156)// left joystick being push right
			{
			ML_1 = 1;	// left motor reverse
			ML_2 = 0;
			MR_1 = 0;	// right motor stop
			MR_2 = 0;
			}
			else
			{
			ML_1 = 1;	// left motor reverse
			ML_2 = 0;
			MR_1 = 0;	// right motor reverse
			MR_2 = 1;
			}			
		}
		
		else
		{
			if(uc_skps_lx < 100) // left joystick being push left
			{
			ML_1 = 1;	// left motor reverse
			ML_2 = 0;
			MR_1 = 1;	// right motor forward
			MR_2 = 0;
			}
			else if (uc_skps_lx > 156)// left joystick being push right
			{
			ML_1 = 0;	// left motor forward
			ML_2 = 1;
			MR_1 = 0;	// right motor reverse
			MR_2 = 1;
			}
			else
			{
			ML_1 = 1;	// left motor brake
			ML_2 = 1;
			MR_1 = 1;	// right motor brake
			MR_2 = 1;
			}			
		}
		if(uc_skps_ry < 75) //right joystik being push up
		{
			if (max_speed < 255) max_speed ++;			
		}
		else if (uc_skps_ry > 180) //right joystik being push down
		{
			if (max_speed > 10) max_speed --;			
		}		
		motor(max_speed, max_speed);	//set the speed for both motor
	
		// vibrator motor on PS2 joystick	
		if (uc_skps(p_circle) == 0)		// if circle is being pressed
		{
		skps_vibrate (p_motor1, 1);		// send command to vibrate right motor
		}
		else skps_vibrate(p_motor1, 0);	// stop motor
		
		if (uc_skps(p_square)== 0)		// if square is being pressed
		{
		skps_vibrate (p_motor2, 200);	// send command to vibrate left motor	
		}
		else skps_vibrate(p_motor2, 0);	// stop motor
	}	

	//wait cross to be released
	while (uc_skps(p_cross) == 0)continue;	
		
	// Display the messages.
	lcd_clr();
	lcd_putstr("  SKPS\n  Done!");
	beep(2);
	delay_ms(2000);
}

// ================================== UART functions =====================================

/*******************************************************************************
* PUBLIC FUNCTION: uart_init
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Initialize the UART module.
*
*******************************************************************************/
void uart_init(void)
{	
	BRG16 = 0;									// Use 8 bit BRG
	SPBRGH = 0;
	BRGH = 1;									// Select high speed baud rate.
	SPBRG = (_XTAL_FREQ / 16 / UART_BAUD) - 1;	// Configure the baud rate.
	SPEN = 1;									// Enable serial port.
	CREN = 1;									// Enable reception.
	TXEN = 1;									// Enable transmission.
	SYNC = 0;									// Asynchronous communication
}



/*******************************************************************************
* PUBLIC FUNCTION: uart_tx
*
* PARAMETERS:
* ~ uc_data		- The data that we want to transmit.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* This function will transmit one byte of data using UART. This is a blocking
* function, if the transmit buffer is full, we will wait until the
* data in the buffer has been sent out before we move in the new data.
*
*******************************************************************************/
void uart_tx(unsigned char uc_data)
{
	// Wait until the transmit buffer is ready for new data.
	while (TXIF == 0);
	
	// Transmit the data.
	TXREG = uc_data;
}



/*******************************************************************************
* PUBLIC FUNCTION: uc_uart_rx
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ The data received from the UART.
*
* DESCRIPTIONS:
* This function will receive one byte of data using UART. This is a blocking
* function because if the receive buffer is empty, we will wait until the
* data is received.
*
*******************************************************************************/
unsigned char uc_uart_rx(void)
{
	// If there is overrun error...
	if (OERR == 1) {
		// Clear the flag by disable and enable back the reception.
		CREN = 0;
		CREN = 1;
	}	
	
	// Wait until there is data available in the receive buffer.
	while (RCIF == 0);
	
	// Return the received data.
	return RCREG;
}



/*******************************************************************************
* PUBLIC FUNCTION: uart_putstr
*
* PARAMETERS:
* ~ csz_string	- The null terminated string to transmit.
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Transmit a string using the UART.
*
*******************************************************************************/
void uart_putstr(const char* csz_string)
{
	// Loop until the end of string.
	while (*csz_string != '\0') {
		uart_tx(*csz_string);
			
		// Point to next character.
		csz_string++;
	}
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


// ==================================== SKPS Functions ===================================
/*******************************************************************************
* PUBLIC FUNCTION: uc_skps_status
*
* PARAMETERS:
* ~ uc_data
*
* RETURN:
* ~ data received from SKPS, the status 
*
* DESCRIPTIONS:
* request SKPS button and joystick status
*
*******************************************************************************/
unsigned char uc_skps(unsigned char uc_data)
{
	// send command to request PS2 status
	uart_tx(uc_data);
	return uc_uart_rx();
}	



/*******************************************************************************
* PUBLIC FUNCTION: skps_vibrate
*
* PARAMETERS:
* uc_motor, uc_value
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* command SKPS to vibrate motor on PS2 controller.
*
*******************************************************************************/
void skps_vibrate(unsigned char uc_motor, unsigned char uc_value)
{
	uart_tx(uc_motor);		//send number of motor, motor 1 or motor 2 to SKPS
	uart_tx(uc_value);		//send the speed, activate or deactivate command to SKPS	
}	


/*******************************************************************************
* PUBLIC FUNCTION: skps_reset
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* reset the SKPS and later release it
*
*******************************************************************************/
void skps_reset(void)
{
	SK_R = 1; 			// reset the SKPS
	__delay_ms(10);		// wait for 10 ms
	SK_R = 0;			// release reset, SKPS back to normal operation
	__delay_ms(10);	
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







