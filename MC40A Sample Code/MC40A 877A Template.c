/*******************************************************************************
* This is the main program to sanity test the MC40A MiniBot Controller using PIC16F877A
*
* Author: Ober Choo Sui Hong @ Cytron Technologies Sdn. Bhd.
*******************************************************************************/
#include <stdio.h>
#include <htc.h>

/*******************************************************************************
* DEVICE CONFIGURATION WORDS FOR PIC16F887                                     *
*******************************************************************************/

 // If PIC16F877A is being selected as device
 __CONFIG(HS &			// External Crystal at High Speed
		 WDTDIS &		// Disable Watchdog Timer.
		 PWRTEN &		// Enable Power Up Timer.
		 BORDIS &		// Disable Brown Out Reset.		 
		 LVPDIS);		// Disable Low Voltage Programming.


// ================================= Define section ===========================================
// Oscillator Frequency.

#define	_XTAL_FREQ		20000000	//using 20MHz external crystal

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
void test_switch(void);
void test_led(void);
void test_dc_motor(void);
void test_adc(void);
void test_uart(void);
void test_limit_switch(void);
void test_LSS05(void);
void test_SKPS(void);
// functions for line following
void motor(unsigned char uc_left_motor_speed,unsigned char uc_right_motor_speed);
void demo_line_follow(void);

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
	
	//led blinking
	/*while(1)
	{
	LED1 = ~LED1;
	delay_ms(100);
	}*/
	
	// Need to make sure the push button is useable 1st.
	test_switch();
		
	while (1) 
		{
		lcd_2ndline();
		lcd_putstr("1+,2=Run");
	
		// Return cursor to the home position.
		lcd_home();
		
		switch (test_no) 
			{
			case 1:
				lcd_putstr("1:All   ");
				if (SW2 == 0) 			// if SW2 is press
				{	while (SW2 == 0);	// waiting for SW2 to be let go
					test_led();
					test_dc_motor();
					test_adc();
					test_uart();
					test_limit_switch();
					test_LSS05();
					test_SKPS();
					demo_line_follow();
				}	
				break;
				
			case 2:
				lcd_putstr("2:LED+BZ");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_led();
				}	
				break;
				
			case 3:
				lcd_putstr("3:DC Mot");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_dc_motor();
				}	
				break;
				
			case 4:
				lcd_putstr("4:ADC   ");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_adc();
				}	
				break;
				
			case 5:
				lcd_putstr("5:UART  ");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_uart();
				}	
				break;
				
			case 6:				
				lcd_putstr("6:LIMIT ");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_limit_switch();
				}	
				break;
				
			case 7:				
				lcd_putstr("7:LSS05 ");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_LSS05();
				}	
				break;	
			case 8:				
				lcd_putstr("8:SKPS  ");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					test_SKPS();
				}	
				break;	
			case 9:				
				lcd_putstr("9:LineFo");
				if (SW2 == 0) 
				{
					while (SW2 == 0);
					demo_line_follow();
				}	
				break;			
		}	
		
		
		
		// If SW1 is pressed...
		if (SW1 == 0) 
		{
			if (++test_no > 9) 
			{
				test_no = 1;
			}				
			while (SW1 == 0);
		}		
	
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
* PRIVATE FUNCTION: test_switch
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the push buttons.
*
*******************************************************************************/
void test_switch(void)
{
	// Display the messages.
	lcd_clr();
	lcd_putstr("Testing \nSwitches");
	delay_ms(1000);	
	
	// Display the messages.
	lcd_clr();
	lcd_putstr("Press \nSW1");
	
	// Waiting for user to press SW1.
	while (SW1 == 1);
	
	// If SW1 is pressed but other switches also become low, trap the error.
	if (SW2 == 0) {
		// Display the error messages and trap the error.
		lcd_clr();
		lcd_putstr(string_SWsError);
		beep(10);
		while (1);
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);	
	beep(1);
	// Display the messages.
	lcd_clr();
	lcd_putstr("Press \nSW2");
	
	// Waiting for user to press SW2.
	while (SW2 == 1);
	
	// If SW2 is pressed but other switches also become low, trap the error.
	if (SW1 == 0 ) 
	{
		// Display the error messages and trap the error.
		lcd_clr();		
		lcd_putstr(string_SWsError);
		beep(10);
		while (1);
	}	
	
	// Waiting for user to release SW2.
	while (SW2 == 0);	
		
	// Display the messages.
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);
	delay_ms(500);
}	

/*******************************************************************************
* PRIVATE FUNCTION: test_led
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the LEDs.
*
*******************************************************************************/
void test_led(void)
{	
	unsigned char i = 0;
	lcd_clr();
	lcd_putstr("Testing \nLED");
	delay_ms(1000);	
	
	// Waiting for user to press SW1.
	while (SW1 == 1) {
	lcd_clr();
	lcd_putstr("Connect \nBuzzer");	//Buzzer and LED share same output pin
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}
	
	lcd_clr();
	lcd_putstr("SW1\nto test");
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);

	// Testing LED 1.
	lcd_clr();
	lcd_putstr("LED1+Buz\n  time/s");
	for(i=0; i<10; i++)
	{
	lcd_goto(0x40);
	lcd_putchar(i+0x30);	
	LED1 = ~LED1; //toggle LED
	delay_ms(500);	
	}
	delay_ms(500);
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);		// done
	delay_ms(500);
}	


/*******************************************************************************
* PRIVATE FUNCTION: test_dc_motor
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the DC Brushed Motor.
*
*******************************************************************************/
void test_dc_motor(void)
{
	unsigned char i;
	unsigned char uc_speed;
	
	// Waiting for user to press SW1.
	while (SW1 == 1) {
	lcd_clr();
	lcd_putstr("Test \nDC Motor");
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}
	
	lcd_clr();
	lcd_putstr("SW1\nto test");
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);	
	
	// Accelerate Left Motor clockwise.
	lcd_clr();
	lcd_putstr("Left Mo \nCW");
	
	ML_1 = 1;
	ML_2 = 0;
	
	for (uc_speed = 40; uc_speed < 255; uc_speed++) {
		set_pwml(uc_speed);
		delay_ms(15);
	}	
	
	// Deaccelerate Left Motor clockwise.
	for (; uc_speed > 40; uc_speed--) {
		set_pwml(uc_speed);
		delay_ms(15);
	}	
	
	// Accelerate Left Motor counter clockwise.
	lcd_clr();
	lcd_putstr("Left Mo \nCCW");
	
	ML_1 = 0;
	ML_2 = 1;
	
	for (uc_speed = 40; uc_speed < 255; uc_speed++) {
		set_pwml(uc_speed);
		delay_ms(15);
	}	
	
	// Deaccelerate Left Motor counter clockwise.
	for (; uc_speed > 40; uc_speed--) {
		set_pwml(uc_speed);
		delay_ms(15);
	}	
	set_pwml(0);
	// Stop motor.
	lcd_clr();
	lcd_putstr("Left Mo \nSTOP!");
	ML_1 = 0;
	ML_2 = 0;
	
	beep(1);
	// Accelerate Right Motor clockwise.
	lcd_clr();
	lcd_putstr("Right Mo\nCW");
	
	MR_1 = 1;
	MR_2 = 0;
	
	for (uc_speed = 40; uc_speed < 255; uc_speed++) {
		set_pwmr(uc_speed);
		delay_ms(15);
	}	
	
	// Deaccelerate Right Motor clockwise.
	for (; uc_speed > 40; uc_speed--) {
		set_pwmr(uc_speed);
		delay_ms(15);
	}	
	
	// Accelerate Right Motor counter clockwise.
	lcd_clr();
	lcd_putstr("Right Mo \nCCW");
	
	MR_1 = 0;
	MR_2 = 1;
	
	for (uc_speed = 40; uc_speed < 255; uc_speed++) {
		set_pwmr(uc_speed);
		delay_ms(15);
	}	
	
	// Deaccelerate Right Motor counter clockwise.
	for (; uc_speed > 40; uc_speed--) {
		set_pwmr(uc_speed);
		delay_ms(15);
	}	
	set_pwmr(0);
	// Stop motor.
	lcd_clr();
	lcd_putstr("Right Mo \nSTOP!");
	MR_1 = 0;
	MR_2 = 0;
	delay_ms(1000);
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);		// done
	delay_ms(500);
}	




/*******************************************************************************
* PRIVATE FUNCTION: test_adc
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the ADC.
*
*******************************************************************************/
void test_adc(void)
{
	//char sz_buffer[35];
	unsigned char uc_d1 = 0, uc_d3 = 0, uc_d4 = 0;
	
	unsigned int ui_adc = 0, uc_d2 = 0;
	unsigned char i = 0;
	// Display the messages.
	lcd_clr();
	lcd_putstr("Testing \nADC");
	delay_ms(1000);	
	
	lcd_clr();		
	
	
	lcd_putstr("ADC:");
	lcd_2ndline();
	lcd_putstr("SW1 exit");
	// Loop until SW1 is pressed.
	// Read from the ADC and display the value.
	ADON= 1; 	//Activate ADC module
	while (SW1 == 1)
	 {	
		ui_adc = 0;	
		for(i = 0; i < 10; i++)
		{
		ui_adc = ui_adc + ui_adc_read();	// read adc value from channel 0
		}
		ui_adc = ui_adc/10;
		
		//extract 4 single digit from uc_adc
		uc_d1 = ui_adc/1000;
		uc_d2 = ui_adc%1000;
		uc_d3 = uc_d2%100;
		uc_d2 = uc_d2/100;	
		uc_d4 = uc_d3%10;
		uc_d3 = uc_d3/10;	
		
		lcd_goto(0x04);	//goto character after ADC:
		lcd_putchar(uc_d1 + 0x30);	//convert digit to ASCII
		lcd_putchar(uc_d2 + 0x30);	//convert digit to ASCII
		lcd_putchar(uc_d3 + 0x30);	//convert digit to ASCII
		lcd_putchar(uc_d4 + 0x30);	//convert digit to ASCII		
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);
	
	ADON = 0;	// Deactivate ADC module
	
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);		// done
	delay_ms(500);
}	


/*******************************************************************************
* PRIVATE FUNCTION: test_uart
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the UART.
*
*******************************************************************************/
void test_uart(void)
{	
	unsigned char i;
	char c_received_data;
		
	// Display the messages.
	lcd_clr();
	lcd_putstr("Testing \nUART");
	delay_ms(1000);
		
	// Waiting for user to press SW1.
	while (SW1 == 1) {
		lcd_clr();
		lcd_putstr("Connect \nUC00A");
		
		for (i = 0; i < 200; i++) {
			if (SW1 == 0) {
				break;
			}	
			delay_ms(10);
		}
		
		lcd_clr();
		lcd_putstr("SW1 \nto test ->");
		
		for (i = 0; i < 200; i++) {
			if (SW1 == 0) {
				break;
			}	
			delay_ms(10);
		}
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);	
	
	// Display the messages.
	lcd_clr();
	lcd_putstr("Enter to\nexit");
		
	// Sending message to the PC.
	uart_putstr("\r\n\nTesting UART...\r\n");
	uart_putstr("Press any key to test.\r\n");
	uart_putstr("Press enter to exit.\r\n\n");
	
	do {
		c_received_data = uc_uart_rx();
		uart_tx(c_received_data);
	}
	while (c_received_data != '\r' && c_received_data != '\n');	
	
	// Sending message to the PC.
	uart_putstr("\r\n\nTest Completed.\r\n");
	
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);		// done
	delay_ms(500);
}	

/*******************************************************************************
* PRIVATE FUNCTION: test_limit_switch
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the limit switches.
*
*******************************************************************************/
void test_limit_switch(void)
{	
	// Display the messages.
	lcd_clr();
	lcd_putstr("Testing \nLIMIT sw");
	delay_ms(1000);	
	
	// Display the messages.
	lcd_clr();
	lcd_putstr("Short \nLIMIT1");
	
	// Waiting for user to press SW1.
	while (LIMIT1 == 1);
	
	// If LIMIT1 is shorted but other Limit switch input also become low, trap the error.
	if (LIMIT2 == 0) {
		// Display the error messages and trap the error.
		lcd_clr();
		lcd_putstr(string_SWsError);
		beep(10);
		while (1);
	}	
	
	// Waiting for user to release SW1.
	while (LIMIT1 == 0);	
	
	beep(1);
	
	// Display the messages.
	lcd_clr();
	lcd_putstr("Short \nLIMIT2");
	
	// Waiting for user to press SW2.
	while (LIMIT2 == 1);
	
	// If LIMT2 is shorted but other Limit switch input also become low, trap the error.
	if (LIMIT1 == 0 ) 
	{
		// Display the error messages and trap the error.
		lcd_clr();		
		lcd_putstr(string_SWsError);
		beep(10);
		while (1);
	}	
	
	// Waiting for user to release SW2.
	while (LIMIT2 == 0);	
		
	// Display the messages.
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);
	delay_ms(500);
}	




/*******************************************************************************
* PRIVATE FUNCTION: test_LSS05
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* Test the LSS05.
*
*******************************************************************************/
void test_LSS05(void)
{	
	unsigned char i;
	// Display the messages.
	lcd_clr();
	lcd_putstr("Testing \nLSS05");
	delay_ms(1000);
	
	// Waiting for user to press SW1.
	while (SW1 == 1) {
		lcd_clr();
		lcd_putstr("Connect \nLSS05");
		
		for (i = 0; i < 200; i++) {
			if (SW1 == 0) {
				break;
			}	
			delay_ms(10);
		}
		
		lcd_clr();
		lcd_putstr("SW1\nto test");
		
		for (i = 0; i < 200; i++) {
			if (SW1 == 0) {
				break;
			}	
			delay_ms(10);
		}
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);
	
	// Display the messages.
	lcd_clr();
	lcd_putstr("SW2 exit");
	delay_ms(1000);
	lcd_2ndline();
		
	// While SW1 is not press, keep reading input from LSS05 and display result on LCD
	while (SW2 == 1) {
	lcd_goto(0x41);		//2nd char on 2nd row	
		if(LEFT == 1) lcd_putchar('X');
		else lcd_putchar(' ');	//display space	
	lcd_goto(0x42);		//2nd char on 2nd row	
		if(M_LEFT == 1) lcd_putchar('X');
		else lcd_putchar(' ');	//display space	
	lcd_goto(0x43);		//2nd char on 2nd row	
		if(MIDDLE == 1) lcd_putchar('X');
		else lcd_putchar(' ');	//display space	
	lcd_goto(0x44);		//2nd char on 2nd row	
		if(M_RIGHT == 1) lcd_putchar('X');
		else lcd_putchar(' ');	//display space	
	lcd_goto(0x45);		//2nd char on 2nd row	
		if(RIGHT == 1) lcd_putchar('X');
		else lcd_putchar(' ');	//display space		
	}	
	
	//wait SW1 to be released
	while (SW2 == 0);	
		
	// Display the messages.
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);
	delay_ms(500);
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

void test_SKPS(void)
{
	unsigned char uc_skps_ru = 0 , uc_skps_lu = 0;
	unsigned char i = 0;
	// Display the messages.
	lcd_clr();
	lcd_putstr("Testing \nSKPS");
	delay_ms(1000);
	
	// Display the messages.
	skps_reset();	//reset the SKPS
	lcd_clr();
	lcd_putstr("SW2 exit");
	delay_ms(1000);
	lcd_2ndline();
	
		
	// While SW1 is not press, keep reading input from LSS05 and display result on LCD
	while (SW2 == 1) {	
		if(!(uc_skps(p_l1) && uc_skps(p_l2) && uc_skps(p_r1) && uc_skps(p_r2)))
		{
			lcd_2ndline();
			lcd_putstr("Buz On  ");
			BUZZER = 1;
		}	
		else 
		{
			BUZZER = 0;
			lcd_2ndline();
			lcd_putstr("       ");
		}

		uc_skps_ru = uc_skps(p_joy_ru);		// read the value of right joystik, up axis
		uc_skps_lu = uc_skps(p_joy_lu);		// read the value of left joystick, up axis
		
		if (uc_skps_ru >5)
		{
		skps_vibrate (p_motor1, 1);		// send command to vibrate right motor
		}
		else skps_vibrate(p_motor1, 0);	// stop motor
		
		if (uc_skps_lu >10)
		{
		skps_vibrate (p_motor2, uc_skps_lu*2);	// send command to vibrate left motor	
		}
		else skps_vibrate(p_motor2, 0);	// stop motor
	}	

	//wait SW1 to be released
	while (SW2 == 0);	
		
	// Display the messages.
	lcd_clr();
	lcd_putstr(string_passed);
	beep(2);
	delay_ms(500);

}

/*******************************************************************************
* PRIVATE FUNCTION: demo_line_follow
*
* PARAMETERS:
* ~ void
*
* RETURN:
* ~ void
*
* DESCRIPTIONS:
* demo the line following. This line following is based on LSS05 , SPG-10-150K ,WL-POL-4610(wheel),
* 7.4V lipo battery (2 cells) for motor and circuit. 
*
*******************************************************************************/
void demo_line_follow(void)
{	
unsigned char i;
while (SW1 == 1) 
	{
	lcd_clr();
	lcd_putstr("  Line \n Follow");	//Buzzer and LED share same output pin
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}
	
	lcd_clr();
	lcd_putstr("  Cal\n LSS05 ");
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}

	lcd_clr();
	lcd_putstr("  SW1\nto start");
	
	for (i = 0; i < 200; i++) {
		if (SW1 == 0) {
			break;
		}	
		delay_ms(10);
	}
	}	
	
	// Waiting for user to release SW1.
	while (SW1 == 0);	
	lcd_clr();
	lcd_putstr("Line fol");
	
	while(SW2 == 1)
	{
		//motor right forward, clockwise looking from right wheel	
		MR_1 = 1;
		MR_2 = 0;
		//motor left forward, counter closkwise looking from left wheel
		ML_1 = 0;
		ML_2 = 1;	
		
		/* Label for LSS05 sensor
		LEFT			RA3
		M_LEFT			RA4
		MIDDLE			RA5
		M_RIGHT			RE0
		RIGHT			RE1	*/		
		if((M_LEFT == 0)&&(MIDDLE == 1)&&(M_RIGHT == 0)) //check middle, middle left and middle right sensor
														//assuming black line, dark ON
		{		
			//motor(100,100);	// robot move straight with both left and right motor moving at same speed
			//motor(uc_left_motor_speed, uc_right_motor_speed)
			motor(200,200);
		}
		
		else if((M_LEFT == 1)&&(MIDDLE == 1)&&(M_RIGHT == 0)) // robot has move to left a little bit
		{			
			//motor(80,100);	// robot turning to left
			motor(120,200);
		}		
		
		else if((M_LEFT == 0)&&(MIDDLE == 1)&&(M_RIGHT == 1)) // robot has move to right a little bit
		{			
			//motor(100,80);	// robot turning to right
			motor(200,120);
		}
		
		else if((LEFT == 0)&&(M_LEFT == 1)&&(MIDDLE == 0))	// robot has move to left
		{			
			//motor(65,90);	// robot turning to Left, hard
			motor(80,180);
		}
				
		else if((MIDDLE == 0)&&(M_RIGHT == 1)&&(RIGHT == 0))// robot has move to right
		{		
			//motor(90,65);	// robot turning to right, hard
			motor(180,80);
		}	
		else if((LEFT == 1)&&(M_LEFT == 1))	// robot has move to left
		{			
			//motor(45,90);	// robot turning to Left, hard
			motor(55,140);
		}
				
		else if((M_RIGHT == 1)&&(RIGHT == 1))// robot has move to right
		{		
			//motor(90,45);	// robot turning to right, hard
			motor(140,55);
		}	
		else if((LEFT == 1)&&(M_LEFT == 0))	// robot has move to the most left side
		{			
			//motor(20,90);	// robot turning to Left, hard
			motor(0,150);
		}
				
		else if((M_RIGHT == 0)&&(RIGHT == 1))// robot has move to the most right site
		{		
			//motor(90,20);	// robot turning to right, hard
			motor(150,0);
		}	
	}//while(SW2 == 1)
	
	//motor right stop	
	MR_1 = 0;
	MR_2 = 0;
	//motor left stop
	ML_1 = 0;
	ML_2 = 0;
	while(SW2 == 0); //wait for SW2 to be released
	
	lcd_clr();
	lcd_putstr("finish!");	
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
	ADCS2 = 0;
	ADCS1 = 1;
	ADCS0 = 0;
	
	// Set AN0 as analog input only, the rest is digital I/O.
	PCFG3 = 1;
	PCFG2 = 1;
	PCFG1 = 1;
	PCFG0 = 0;

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







