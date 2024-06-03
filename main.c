#include <atmel_start.h>
#include "util/delay.h"
#include <stdio.h>
#define sbi(PORTX, BitX) (PORTX |= (1<<BitX))  // SET BIT
#define cbi(PORTX, BitX) (PORTX &= ~(1<<BitX)) // CLEAR BIT

void putchar1_TX_int(unsigned long data);
unsigned long ReadCout(void);

volatile unsigned long weight = 0;
volatile unsigned long weight_before = 0;
volatile unsigned long weight_after = 0;
volatile unsigned long weight_change = 0;
float injection_rate = 0;
volatile unsigned long TimeForSpeed=0;

volatile unsigned long offset = 0;
volatile unsigned long k = 0;
volatile int offset_flag = 3;
volatile float calibration_factor = 0.0037; // 초기 캘리브레이션 팩터


/*시간 측정을 위해 추가한 변수
volatile unsigned long start_time = 0;
volatile unsigned long stop_time = 0;
volatile unsigned int start_flag = 0;
volatile unsigned int stop_flag = 0;
volatile unsigned long total_time = 0;
*/
volatile unsigned int overflow_count = 0;
volatile unsigned int timer_running = 0; //타이머 실행 여부를 위한 변수
volatile unsigned int measured_time = 0;
volatile unsigned int overflow_count_sec = 0;
volatile unsigned int weight_flag = 0;
volatile unsigned int TT = 0;
volatile unsigned int Speed = 0;
void putstring(char* str);

void timer0_init(){
	TCCR0 = 0b00000111; //분주비를 1024로
	TIMSK = (1 << TOIE0);
	sei();
}

//타이머 실행 ISR
ISR(TIMER0_OVF_vect){ //오버플로우 인터럽트가 걸릴 때마다 실행된다.
	if(timer_running){ //만일 타이머가 작동된다면
		overflow_count++;
		if(overflow_count >= 61.27){ 
			overflow_count_sec++; //1초의 시간이 흘렀다
			TimeForSpeed = overflow_count_sec - TT*60;
			overflow_count = 0; // 초기화시켜주기
			if(overflow_count_sec >= 61){
				TT = TT+1;
				TimeForSpeed = 0;
			}
		}
	}
}
//switch 추가
void switch_setting()
{	
	DDRE = 0b00110000;
	PINE = 0xff; //초기에는 전부 off

}

void addMore_switch_setting(){
	DDRA = 0b11111100; // A0, A1 에 풀업 스위치, 입력으로 설정
	PORTA |= 0b00000011; //풀업 저항 활성화
}


void putchar0(char c) // for Debug // 115200bps
{
	while(!(UCSR0A & 0x20));
	UDR0 = c;
}

void putchar1(char c) // for Bluetooth(HC-06) // 9600bps
{
	while(!(UCSR1A & 0x20));
	UDR1 = c;
}

void putstring(char* str){
	while(*str){
		putchar1(*str++);
	}
}

int main(void)
{
	//timer
	timer0_init();

	//switch on setting
	switch_setting();

	//add more switch setting
	addMore_switch_setting();

	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	DDRC = 0b00000010; // C0(DOUT):input, C1(SCK):output

	// config for USART0 (115200bps)
	UBRR0H = (8 >> 8); // '0'
	UBRR0L = 8;

	UCSR0B = (1 << TXEN0); // 0x08;
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 0x06;

	// config for USART1 (9600bps)
	UBRR1H = (103 >> 8);
	UBRR1L = 103;

	UCSR1B = (1 << TXEN1); // 0x08;
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // 0x06;

	/* Replace with your application code */
	while (1) {
		//E4핀이 눌리면 0점을 맞춘다
		if (PINE == 0b11101111){
			offset_flag = 1;
		}
		//E5핀이 눌리면
		if (PINE == 0b11011111){
			offset_flag = 2;
		}

		//A0핀이누르면 START!
		if((PINA & 0b00000001) == 0){
			overflow_count = 0;
			overflow_count_sec = 0;
			timer_running = 1;
			putstring("START\n");
				}

		if(timer_running == 1){
			if(weight_flag == 0){
				weight_before = weight;
				weight_flag = 1;
			}
			if(weight_flag == 1){
				if(TimeForSpeed == 60){
						weight_after = weight;
						weight_change = weight_after - weight_before;
						Speed = weight_change/60;
						putstring("Speed = ");
						putchar1_TX_int(Speed);
						putstring(" g/sec \n");
						weight_flag = 0;
					}
			}
			
		}
						
		//A1 핀이 눌리면 STOP!
		if((PINA & 0b00000010) == 0){
			if(timer_running){
				timer_running =  0;
				measured_time = overflow_count_sec;								
				putstring("STOP\n");
				putstring("Measured time : ");
				putchar1_TX_int(measured_time);
				overflow_count_sec =0;
				putstring(" seconds\n");
			}
		}


		weight = ReadCout() / 4;
		weight = (unsigned long)(weight * calibration_factor);
		weight_before = weight;
		
		putchar1_TX_int(weight);
		putchar1('\r');
		putchar1('\n');
		PORTB ^= 0x01;
		_delay_ms(500);
	}
}

unsigned long ReadCout(void)
{
	unsigned long sum = 0, count = 0, data1 = 0, data2 = 0;

	for (int j = 0; j < 10; j++)
	{
		sbi(PORTC, 0); // DOUT : 1
		cbi(PORTC, 1); // SCK : 0

		count = 0;

		while((PINC & 0b00000001) == 0b00000001);

		for (int i = 0; i < 24; i++)
		{
			sbi(PORTC, 1); // SCK:1
			count = count << 1;
			cbi(PORTC, 1); // SCK:0
			if ((PINC & 0b00000001) == 0b00000001)
			count++;
		}
		sbi(PORTC, 1); // SCK:1
		count = count^0x800000;
		cbi(PORTC, 1); // SCK:0

		sum += count;
	}
	data1 = sum / 10;

	if (offset_flag == 3){
		offset = data1;
		k = data1;
		offset_flag = 0;
	}

	if (offset_flag == 1){
		offset = data1;
		offset_flag = 0;
	}

	if (data1 > offset)
	data2 = data1 - offset;
	else
	data2 = 0;

	if (offset_flag == 2){
		offset = k;
		offset_flag = 0;
	}

	return data2;
}

void putchar1_TX_int(unsigned long data)
{
	unsigned long temp = 0;

	temp  = data / 10000;
	putchar0(temp+48);
	putchar1(temp+48);
	temp = (data%10000)/1000;
	putchar0(temp+48);
	putchar1(temp+48);
	temp = (data%1000)/100;
	putchar0(temp+48);
	putchar1(temp+48);
	temp = (data%100)/10;
	putchar0(temp+48);
	putchar1(temp+48);
	temp = data%10;
	putchar0(temp+48);
	putchar1(temp+48);
}
