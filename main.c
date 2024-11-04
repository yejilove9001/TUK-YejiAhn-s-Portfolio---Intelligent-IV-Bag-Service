//결선에 관한 부분 정리
//A0 A1 : start, stop
//E4 E5 : 영점, 영점 초기화
//hc05  DT : PD3 , ST : PD2
//hx711 DOUT C1, SCk C0


#include <avr/io.h>
#include <atmel_start.h>
#include <util/delay.h>
#define __DELAY_BACKWARD_COMPATIBLE__

#define  sbi(PORTX, BitX) (PORTX |= (1<<BitX))  // SET BIT
#define  cbi(PORTX, BitX) (PO`RTX &= ~(1<<BitX)) // CLEAR BIT

void putchar1_TX_int(unsigned long data);
unsigned long ReadCout(void);

volatile unsigned long weight = 0;
volatile unsigned long offset = 0;
volatile int offset_flag = 0;
volatile int k;

volatile float calibration_factor = 0.0060;

volatile int timer_running = 0;
volatile int overflow_count = 0;
volatile int sec = 0;
volatile int TimeForSpeed = 0;
//volatile int TT = 0;
volatile int weight_flag = 0;
volatile int weight_before = 0;
volatile int weight_after = 0;
volatile int weight_change = 0;
volatile int Speed = 0;
volatile int RemainingTime = 0;
volatile int measured_time = 0;

//buzzer
volatile int buzzer_flag = 0;
/*
void buzzer(int hz, int count);
void buzzer(int hz, int count){
   int i;
   double ms;
   ms = 500.00/hz;
   for(i=0; i<count; i++){
      PORTB = 0b00010000;
      _delay_ms(ms);
      PORTB = 0b00000000;
      _delay_ms(ms);
   }
   
}*/




void timer_init(){
   TIMSK = 0x01; // timer overflow interrupt enable
   TCCR0 = 0x06; // Normal, 256분주비
   TCNT0 = 50;
   sei();

}


//오버플로우 인터럽트 서비스 루틴
ISR(TIMER0_OVF_vect) {
   if (timer_running) {
      overflow_count++;
      TCNT0 = 50;
      if (overflow_count == 305*5) {  // 5초 경과
         sec++;
         overflow_count = 0;         
         weight_flag = 1;
              
         
      }
   }
}

void buzzer_init(){
   DDRB = 0b00010000;
}

/*
void buzzer(int hz, int count){
   int i;
   double ms;
   ms = 500.00/hz;

   for(i=0; i<count; i++){
      PORTB = 0b00010000;
      _delay_ms(ms);
      PORTB = 0b00000000;
      _delay_ms(ms);
   }
   
}
*/
//buzzer
/*
ISR(INT2_vect){
   if(buzzer_flag==1){
      int i = 0;
      for(i=0; i<20; i++){
         buzzer(480,12);
         buzzer(320, 8);
      }
   }
}*/


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

void putstring(char* str){ //문장출력
   while(*str){
      putchar1(*str++);
   }
}

//Start, Stop 스위치
void start_stop_switch(){
   DDRA =  0b11111100;
   PORTA |= 0b00000011;
}

//zeropoint switch
void zeropoint_switch(){
   DDRE = 0b00110000;
   PINE = 0xff; // off
}

int main(void)
{
   sei();
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();
    DDRC = 0b00000010; // C0(DOUT):input, C1(SCK):output

    //int i;
    //char data[] = "Hello-Atmega128~\r\n";
    //char data1[] = "Hello-Bluetooth~\r\n"; // 0x0d, 0x0a

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
    
// 타이머 관련
   timer_init();

// 스위치 세팅
   start_stop_switch();
   zeropoint_switch();

//buzzer
   buzzer_init();
        
    /* Replace with your application code */
    while (1) {
      
        weight = ReadCout() / 4;
        weight = weight * calibration_factor;
        putchar1_TX_int(weight);
        putchar1 ('\r');
        putchar1 ('\n');
        
        PORTB ^= 0x01;
        _delay_ms(500);
        
      if(PINE == 0b11101111){
         offset_flag = 1;
      }
      

        //i = 0;
        // to usart0 (115200bps)
        //while ( data[i] != '\0') // NULL
        //putchar0(data[i++]);
        
        // to usart1 (9600bps)
        //i = 0;
        //_delay_ms(500);
        //while ( data1[i] != '\0') // NULL
        //putchar1(data1[i++]);



      if((PINA & 0b00000001) == 0){
         timer_running = 1; //ISR Start
         putstring("START\n");
         }

         if(weight_flag == 0){
            weight_before = weight;
            weight_flag = 2; //아무것도 못 걸리게.
            }
         if(weight_flag == 1){ //overflow interrupt 가동 시 1
            weight_after = weight;
            weight_change = weight_before - weight_after;
            if(weight_change < 0){
               weight_change = 0;
               Speed = 0;
            }
            
            
            else{
               Speed = (float)weight_change / 5.0;               
            }

         
         if(Speed == 0){
            RemainingTime = weight_after / 1;
         }
         else{
            RemainingTime = weight_after / Speed; 
            }
                        
            putstring("weight before : ");
            putchar1_TX_int(weight_before);
            putstring("\n");
            putstring("Debug: weight_after = ");
            putchar1_TX_int(weight_after);
            putstring("\n");
            putstring("weight change");
            putchar1_TX_int(weight_change);
            putstring("\n");
            putstring("Speed = ");
            putchar1_TX_int(Speed);
            putstring(" g/sec \n");
            putstring("Time Remaining : ");
            putchar1_TX_int(RemainingTime);
            putstring(" sec \n");


            if(Speed > 3){
               int i,j,k = 0;
               for(i=0; i<20; i++){
                  for(j=0; j<12; j++){
                     PORTB = 0b00010000;
                     _delay_ms(1.04);
                     PORTB = 0b00000000;
                     _delay_ms(1.04);
                  }
                  for(k=0; k<8; k++){
                     PORTB = 0b00010000;
                     _delay_ms(1.05);
                     PORTB = 0b00000000;
                     _delay_ms(1.05);
                  }
               }
            }

            weight_flag = 0;
         }
      

      //A1 핀이 눌린 경우
      if((PINA & 0b00000010) == 0){
         if(timer_running){
            measured_time = sec;
            putstring("STOP\n");
            putstring("Measured time : ");
            putchar1_TX_int(measured_time);
            putstring("  seconds \n");

            overflow_count = 0;
            sec = 0;
            timer_running = 0;
            weight_flag = 0;
         }
      }

   }//while 꺽새
}
unsigned long ReadCout(void)
{
    unsigned long sum = 0, count = 0, data1 = 0, data2 = 0;
    
    for (int j = 0; j < 10; j++)
    {
        sbi(PORTC, 0); // DOUT : 1
        cbi(PORTC, 1); // SCK : 0
        
        count = 0;
        
        while( (PINC & 0b00000001) == 0b00000001);
        
        for ( int i = 0; i < 24; i++)
        {
            sbi(PORTC, 1); // SCK:1
            count = count << 1;
            cbi(PORTC, 1); // SCK:0
            if ( (PINC & 0b00000001) == 0b00000001)
            count++;
        }
        sbi(PORTC, 1); // SCK:1
        count = count^0x800000;
        cbi(PORTC, 1); // SCK:0
        
        sum += count;
        //putchar0(sum); // for test
    }
    data1 = sum/10;
    
    if ( offset_flag == 0) //초기의 경우
    {
        offset = data1;
        offset_flag = 4; // offset의 변경 중단      
    }

   if(offset_flag == 1){ //영점 스위치를 누른 경우
      offset = data1;
      offset_flag = 4; //offset 변경 중단
   }

    
    if (data1 > offset)
    data2 = data1 - offset;
    else
    data2 = 0;

    
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
