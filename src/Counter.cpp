 
#define Z_C 2//pin of zero cross
#define PWM 11 

#define MAX_PERIOD_16 (1000000UL * 1024UL / F_CPU * 65536UL)	// 4194304 (0.24 Гц) на 16 МГц
#include <Arduino.h>

int dimmer;
  
void setup (){
  pinMode(Z_C,INPUT_PULLUP); //setup ZeroCross pinMode
  pinMode(PWM,OUTPUT); // setup PWM pinMode
  
  EICRA = (1<<ISC01) | (1<<ISC00);// rising
  EIMSK |= (1<<INT0);// enable INT0 interrupts(PD2)
  
  TIMSK1 |= (1<<OCIE1A);  // enable OCR1A interrupt

}

void loop(){
  dimmer = map(analogRead(0), 0, 1023, 500, 9300);
}

ISR(INT0_vect){
  digitalWrite(PWM,0); // switch off triac
//====================SET PERIOD===================
  uint32_t _period = dimmer; 
  _period = constrain(_period,1, MAX_PERIOD_16);
  uint32_t _cycles = F_CPU/1000000*_period;//calculations of nuber cycles per period
  uint8_t prescaler = 0x00;
  uint8_t divider = 0x00;

//==========Select prescaler=========
if(_cycles<65536UL){
  prescaler = 0x01;
  divider = 1UL; 
} else if (_cycles < 65536UL * 8) {
  prescaler = 0x02;
  divider = 8UL;
} else if (_cycles < 65536UL * 64) {
  prescaler = 0x03;
  divider = 64UL;
} else if (_cycles < 65536UL * 256) {
  prescaler = 0x04;
  divider = 256UL;
} else {
  prescaler = 0x05;
  divider = 1024UL;
}

uint16_t top = (_cycles < 65536UL * 1024 ? (_cycles / divider) : 65536UL) ;
TCCR1A = (TCCR1A & 0xF0);
TCCR1B = ((1 << WGM13) | (1 << WGM12) | prescaler);   // CTC mode + set prescaler
OCR1A = top - 1; // Set timer top
uint8_t clock = 0x00;
clock = (TCCR1B & 0x07); 
return (1000000UL / ((F_CPU / divider) / top));   // Return real timer period
}

ISR(TIMER1_COMPA_vect){
  digitalWrite(PWM,1);
  TCCR1B = (TCCR1B & 0xF8);
  TCNT1 = 0x00;
  
}



