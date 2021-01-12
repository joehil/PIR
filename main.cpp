#include <Arduino.h>
#include <RCSwitch.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

unsigned long motion = 0;
volatile unsigned int count = 99;

#define MOTION_INT 2
#define RF_DATA 8
#define RF_VCC 7
#define RF_GND 9
#define LED 13


ISR(WDT_vect)
  /* Watchdog Timer Interrupt Service Routine */
  {
    count++;
  }

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);

  ADCSRA = ADCSRA & B01111111; // ADC abschalten, ADEN bit7 zu 0
  ACSR = B10000000; // Analogen Comparator abschalten, ACD bit7 zu 1
  DIDR0 = DIDR0 | B00111111; // Digitale Eingangspuffer ausschalten, analoge Eingangs Pins 0-5 auf 1

  /* Setup des Watchdog Timers */
  MCUSR &= ~(1<<WDRF);             /* WDT reset flag loeschen */
  WDTCSR |= (1<<WDCE) | (1<<WDE);  /* WDCE setzen, Zugriff auf Presclaler etc. */
  WDTCSR = 1<<WDP0 | 1<<WDP3;      /* Prescaler auf 8.0 s */
  WDTCSR |= 1<<WDIE;               /* WDT Interrupt freigeben */

  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(RF_DATA);
  
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);

  // Optional set pulse length.
  mySwitch.setPulseLength(320);
  
  // Optional set number of transmission repetitions.
  mySwitch.setRepeatTransmit(3);

  pinMode(MOTION_INT, INPUT);

  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(RF_GND, OUTPUT);
  pinMode(RF_VCC, OUTPUT); 
  pinMode(RF_DATA, OUTPUT); 
  
  digitalWrite(RF_VCC, LOW);
  digitalWrite(RF_GND, LOW);
  digitalWrite(RF_DATA, LOW);

  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  delay(2000);
  digitalWrite(LED, LOW);
}

void loop() {
  if (count > 15) {
    motion = digitalRead(MOTION_INT);
    if (motion > 0){
      digitalWrite(RF_VCC, HIGH);
      mySwitch.send((signed long)-1073741824 + motion,32); // 1879048192 legt das Device fest}
    }
    count = 0;
    digitalWrite(RF_VCC, LOW);

    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
  }
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  
  power_usart0_disable();
  cli(); // deactivate interrupts
  sleep_enable(); // sets the SE (sleep enable) bit
  sei(); // 
  sleep_cpu(); // sleep now!!
  power_all_enable();
  sleep_disable(); 
}
