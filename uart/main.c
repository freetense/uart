#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#define LED0 PB0 // OC0A
#define LED1 PB1 // OC0B

volatile uint8_t uart;
uint8_t temp;
volatile uint8_t count;
volatile uint8_t start;
volatile uint8_t c;
volatile uint8_t uart_data;
volatile uint8_t Rece_bit;
volatile uint8_t rec;
volatile uint8_t usart_r;
volatile uint8_t coef;

ISR(INT0_vect){
rec=1;} // Прерывание чисто для определения стартового бита при приеме,
// используется редко, можно сюда повесить что-либо еще

ISR(TIM0_COMPA_vect){
	TIMSK0=0x00;
	TCCR0B=0x00;   // Единственный Таймер, используется для формирования четких промежуток
	OCR0A=0x7D;       // между битами, как при приеме так и при передачи
	c=1;
	TCNT0=0;
	TIMSK0=0x04;
	TCCR0B=0x02;
	// Значение "сброс при совпадении" загружается каждый раз из переменной
	OCR0A=0x7D;      // Можно быстро менять скорости UART
	Rece_bit=1;
}

int lov (uint8_t data2) {
	if (count>=8){
		PORTB|=(1<<4); start=0; temp=0; c=0; count=0;
	TIMSK0=0; goto nah;}
	
	if(c==1){
		if (start==0){temp=0x80; start=1;
			count--; goto razvet;
		}
		temp=data2;
		temp=temp>>count;
		temp=temp<<7;
		razvet:
		switch(temp){
			case 0x80 : PORTB&=~(1<<4);  break;
			case 0x00 : PORTB|=(1<<4);   break;
		}
		count++; c=0;
	}
	nah:;
}

int UART_trans(uint8_t data){
	uint8_t f;
	data=~data;
	TIMSK0=0x04;
	TCCR0B=0x02;
	for(f=0;f<10;f++){
		while(c==0);
		lov(data);
	}
	start=0; temp=0; c=0; count=0;
	TIMSK0=0; 
}

int UART_receiv(void){
	uint8_t a;
	usart_r=0;
	
	MCUCR=0x02; // INT0 Interrupt
	GIMSK=0x40; // INT0 Interrupt
	
	while(rec==0); // Ждать, пока не случится стартовый бит
	MCUCR=0; // INT0 Interrupt
	GIMSK=0; // INT0 Interrupt
	TIMSK0=0x04;
	TCCR0B=0x02;
	rec=0;
	for(a=0; a<9; a++){
		while(Rece_bit==0);

		if(bit_is_set(PINB,3)){usart_r |=(1<<7);} else {usart_r &=~(1<<7);}
		usart_r=usart_r>>1;
		Rece_bit=0;
	}
}

int main(void)
{
		
		DDRB |= (1 << LED0)|(1 << LED1);
		PORTB &= ~((1 << LED0)|(1 << LED1));
		
		TCCR0A = 0xB3;
		TCCR0B = 0x02;
		TCNT0=0;
		OCR0A=0;
		OCR0B=0;
		
	DDRB&=~(1<<3); //
	DDRB|=(1<<4);  //
	
	asm("sei");
  while(1)
  {
	  do // Нарастание яркости
	  {
		  OCR0A++;
		  OCR0B = OCR0A;
		  _delay_ms(5);
	  }
	  while(OCR0A!=255);
	  _delay_ms(1000); // Пауза 1 сек.
	  do // Затухание
	  {
		  OCR0A--;
		  OCR0B = OCR0A;
		  _delay_ms(5);
	  }
	  while(OCR0A!=0);
	  _delay_ms(1000); // Пауза 1 сек.
	  
	   UART_trans('S');  // Отправляем обратно
  }

}