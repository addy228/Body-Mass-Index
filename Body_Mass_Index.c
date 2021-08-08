/*
 * Body_Mass_Index.c
 *
 * Created: 07-02-2018 PM 06:19:40

 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#define  Trigger_pin	PC0	//Trigger pin

int TimerOverflow = 0;


void cmd_4bit(unsigned char cmd)
{
	PORTD =cmd;
	PORTC= 0b00000100;
	_delay_ms(1);
	PORTC=0b00000000;
	_delay_ms(1);
	
	PORTD=cmd<<4;
	PORTC= 0b00000100;
	_delay_ms(1);
	PORTC=0b00000000;
	_delay_ms(1);
}

void data_4bit(unsigned char data)
{
	PORTD=data;
	PORTC=0b00000110;
	_delay_ms(1);
	PORTC=0b00000010;
	_delay_ms(1);

	PORTD=data<<4;
	PORTC=0b00000110;
	_delay_ms(1);
	PORTC=0b00000010;
	_delay_ms(1);
}

void LCD_String (char *str)							/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)							/* Send each char of string till the NULL */
	{
		data_4bit (str[i]);							/* Call LCD data write */
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD function */
{
	if (row == 1)
	cmd_4bit((pos & 0x0F)|0x80);					/* Command of first row and required position<16 */
	else if (row == 2)
	cmd_4bit((pos & 0x0F)|0xC0);					/* Command of Second row and required position<16 */
	else if (row == 3)
	cmd_4bit((pos & 0x0F)|0x94);
	else if (row == 4)
	cmd_4bit((pos & 0x0F)|0xD4);
	LCD_String(str);								/* Call LCD string function */
}

void lcd_init()
{
	cmd_4bit(0x02);
	cmd_4bit(0x28);
	cmd_4bit(0x0e);
	cmd_4bit(0x06);
	cmd_4bit(0x01);
	cmd_4bit(0x80);
}

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;								/* Increment Timer Overflow count */
}

void ADC_init(void)
{
	ADCSRA |= 1<<ADPS2 | 1<<ADPS1;
	ADMUX  |= 1<<REFS0;
	ADMUX  |= 1<<MUX2| 1<<MUX0;
	ADCSRA |= 1<<ADEN;
	ADCSRA |= 1<<ADIE;
	ADCSRA |= 1<<ADSC;
}

ISR(ADC_vect)
{
	ADCSRA |= 1<<ADSC;
}

void code_init()
{
	cmd_4bit(0x01);
	LCD_String_xy(1,5,"Welcome to");
	_delay_ms(500);
	LCD_String_xy(2,3,"BMI Calculator");
	_delay_ms(1000);
	cmd_4bit(0x94);
	LCD_String("Please Stand Steady");
	_delay_ms(5000);
	_delay_ms(5);
	cmd_4bit(0xD4);
	LCD_String("Press the Button");
}

void co_init()
{
	char string[10];
	long count;
	double distance;
	int a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,avg;

 	while(bit_is_clear(PINB, 1))
	{
		a=ADCW;
		_delay_ms(2);
		b=ADCW;
		_delay_ms(2);
		c=ADCW;
		_delay_ms(2);
		d=ADCW;
		_delay_ms(2);
		e=ADCW;
		_delay_ms(2);
		f=ADCW;
		_delay_ms(2);
		g=ADCW;
		_delay_ms(2);
		h=ADCW;
		_delay_ms(2);
		i=ADCW;
		_delay_ms(2);
		j=ADCW;
		_delay_ms(2);
		k=ADCW;
		_delay_ms(2);
		l=ADCW;
		_delay_ms(2);
		m=ADCW;
		_delay_ms(2);
		n=ADCW;
		_delay_ms(2);
		o=ADCW;
		_delay_ms(2);
		p=ADCW;
		_delay_ms(2);
		q=ADCW;
		_delay_ms(2);
		r=ADCW;
		_delay_ms(2);
		s=ADCW;
		_delay_ms(2);
		t=ADCW;
		_delay_ms(2);

		avg=(a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r+s+t)/20;

		cmd_4bit(0x01);

		double tenbit =((double) (avg/1023)*80;
		dtostrf(tenbit, 2, 2, string);
		LCD_String_xy(1,0,"Weight = ");
		LCD_String_xy(1,9, string);
		cmd_4bit(0x8F);
		LCD_String(" Kg");

		PORTC |= (1 << Trigger_pin);		/* Give 10us trigger pulse on trig. pin to HC-SR04 */
		_delay_us(10);
		PORTC &= (~(1 << Trigger_pin));

		TCNT1 = 0;							/* Clear Timer counter */
		TCCR1B = 0x41;						/* Setting for capture rising edge, No pre-scaler*/
		TIFR = 1<<ICF1;						/* Clear ICP flag (Input Capture flag) */
		TIFR = 1<<TOV1;						/* Clear Timer Overflow flag */

		/*Calculate width of Echo by Input Capture (ICP) on PortD PB0*/

		while ((TIFR & (1 << ICF1)) == 0);	/* Wait for rising edge */
		TCNT1 = 0;							/* Clear Timer counter */
		TCCR1B = 0x01;						/* Setting for capture falling edge, No pre-scaler */
		TIFR = 1<<ICF1;						/* Clear ICP flag (Input Capture flag) */
		TIFR = 1<<TOV1;						/* Clear Timer Overflow flag */
		TimerOverflow = 0;					/* Clear Timer overflow count */

		while ((TIFR & (1 << ICF1)) == 0); 	/* Wait for falling edge */
		count = ICR1 + (65535 * TimerOverflow);			/* Take value of capture register */
		/* 8MHz Timer freq, sound speed =343 m/s, calculation mentioned in doc. */

		distance =201.3-((double) count/466.47);

		dtostrf(distance, 2, 2, string);	/* Convert distance into string */
		LCD_String_xy(2,0, "Height = ");
		LCD_String_xy(2,9,string);			/* Print distance on LDC*/
		cmd_4bit(0xCF);
		LCD_String(" cm");

		double BMI=(tenbit/(distance*distance))*10000;
		dtostrf(BMI, 2, 2, string);
		cmd_4bit(0x94);
		LCD_String("BMI = ");
		cmd_4bit(0x9A);
		LCD_String(string);

		if ((18<BMI)& (BMI<24))
		{
			cmd_4bit(0xD4);
			LCD_String("You are Healthy");
		}

		else if (BMI>24)
		{
			cmd_4bit(0xD4);
			LCD_String("You are OverWeight");
		}

		else if (BMI<18)
		{
			cmd_4bit(0xD4);
			LCD_String("You are UnderWeight");
		}
	}
}

int main(void)
{
	DDRD=0b11110000;
	DDRC=0b00000111;
	DDRB=0b00000000;
	PORTB = 0xFF;			/* Turn on Pull-up */
	
	lcd_init();
	ADC_init();
	code_init();

	sei();					/* Enable global interrupt */
	TIMSK = (1 << TOIE1);	/* Enable Timer1 overflow interrupts */
	TCCR1A = 0;				/* Set all bit to zero Normal operation */
	
    while(1)
    {
		co_init();

		while(bit_is_clear(PINB, 1))
		{
			co_init();  
		}
    }
}