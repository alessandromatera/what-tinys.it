#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define A4 button

unsigned int seconds = 0;
unsigned int hours = 0;

boolean button_interrupt = false; //boolean to let you know when interrupt button is occurred

int setupPeriod = 2; //hold down button for this period in seconds to enter in programming mode

//overflow interrupt from 32.768Khz Crystal every 1 sec
ISR(TIMER2_OVF_vect){
  //increase the seconds
  seconds++;
  //if seconds is over 3600, we increase the hours
  if (seconds > 3600) {
    seconds = 0;
    hours++;
    //if hours is over 11 come back to zero
    if (hours > 11) {
      hours = 0;
    }
  }
}

//pin-change interrupt from button
ISR(PCINT1_vect){
  //disable interrupt button to not start another interrupt
  bitClear(PCMSK1,PCINT12);
  button_interrupt = true;
}


void setup() {
  TIMSK2  = 0; //stop timer interrupt
  TCCR2A = 0; // this stops the timer
  TCCR2B = 0;  // this stops the timer
  TCNT2 = 0;  // reset the timer2 counter

  //Enable asynchronous mode
  bitSet(ASSR, AS2); //set TMR2 to work in asynchronous mode
  TCCR2B = bit(CS22) | bit(CS20); // prescaler 128 -> 256 Hz = 1 overflow/sec
  bitSet(TIMSK2, TOIE2); //eable overflow interrupt for tmr2

    //setup pin-change interrupt for button
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);//enable pullup resistor
  bitSet(PCMSK1,PCINT12); //enable pinchange for pin A4
  bitSet(PCICR,PCIE1); //enable interrupt button

  //disabling all unnecessary peripherals to reduce power
  ADCSRA &= ~bit(ADEN); //disable ADC
  power_adc_disable(); // disable ADC converter
  power_spi_disable(); // disable SPI
  power_usart0_disable();// disable Serial (USART)
  power_timer1_disable();// disable Timer 1
  power_twi_disable(); // disable TWI (I2C)

}

void loop() {
  //if we press the button the pin-change interrupt occur
  //then if we fast press the button show up the time
  //else, if we press the button for "setupPeriod" seconds we
  //enter in programming mode and then set the time
  if (button_interrupt){
    button_interrupt = false; //resetting our boolean flag
    
    //store and calculating the pressing time of the button
    int oldSeconds = seconds;
    int buttonSeconds = 0;
    
    //while the button is pressed we count the pressing time
    while(!digitalRead(button)){
      buttonSeconds = seconds-oldSeconds;
      if (buttonSeconds > setupPeriod){ //if we have pressed for over setupPeriod seconds
        setClock();  //enter in programming mode
      }
    }
    
    //show up the time
    showTime();

    bitSet(PCMSK1,PCINT12); //enable interrupt button
  }

  goSleep(); //go power save
}


void showTime(){

  int showPeriod = 4000; //showing time period in milliseconds
  int blinkingPeriod = 500; //blinking period for minutesLED in milliseconds

  int hoursLED = calculateHoursLED(hours, seconds);
  int minutesLED = calculateMinutesLED(seconds);

  //animation light game
  int oldSeconds = seconds;
  int i_led_game = hoursLED+1;
  if (i_led_game > 35)
    i_led_game = 0;
  boolean minutesLEDfired = false;
  unsigned long previouosMillis_animation = millis();
  while(i_led_game != hoursLED){
    changeLED(hoursLED, 1); //turn On hours LED
    changeLED(hoursLED, 0); //turn off hoursLED for multiplexing

    changeLED(i_led_game, 1); //turn on game LED
    changeLED(i_led_game, 0);

    //if LED game pass on minutes LED turn on minutesLED and stay on
    if(i_led_game == minutesLED || minutesLEDfired){ 
      minutesLEDfired = true;
      changeLED(minutesLED, 1); //turn On minutes LED
      changeLED(minutesLED, 0);
    }

    if ((millis() - previouosMillis_animation) > 10){ //every 10 ms change the game LED
      previouosMillis_animation = millis();
      i_led_game++;
      if (i_led_game > 35)
        i_led_game = 0;
    }
  }


  unsigned long currentMillis = millis();
  unsigned long previousMillis = currentMillis;
  unsigned long previousMillis_blinking = currentMillis;
  boolean minutesLEDstate = true;

  //show-up hours and seconds
  while ((millis() - previousMillis) < showPeriod) {

    changeLED(hoursLED, 1); //turn On hours LED
    changeLED(hoursLED, 0); //turn Off hours LED
    if ((millis() - previousMillis_blinking) >= blinkingPeriod) { //blink minute LED
      previousMillis_blinking = millis();
      minutesLEDstate = !minutesLEDstate;
    }
    changeLED(minutesLED, minutesLEDstate); //turn On/Off minutes LED
    changeLED(minutesLED, 0); //turn Off minutes LED
  }
}

//change hours and minutes -> programming mode
void setClock(){

  //while the button is pressed we turn on all LED
  //to let you know that we are in programming mode
  while(!digitalRead(button)){
    for (int i = 0; i <36; i++){
      changeLED(i, 1);
      changeLED(i, 0);
    }
  }


  int oldSeconds = seconds; //variable for button timing

  boolean hourSetted = false; //boolean for exit hours programming mode
  int hoursToSet = hours; //store hours
  if (seconds>=3550) //if seconds is higher than 3550 we turn on next hour because LED 0 is turned on
    hoursToSet++;
  int hoursLED = calculateHoursLED(hoursToSet, 0); //calculatin which hoursLED need to turnON
  changeLED(hoursLED, 1); //turn on the hoursLED
  while(!hourSetted){ //while hours are not setted
    boolean buttonPressed = false;
    oldSeconds = seconds;

    //if we press the button for small period of time, we change the hours
    //if we press for long time (setupPeriod) we set the hours and then go to setting minutes
    while(!digitalRead(button)){ 
      buttonPressed = true;
      delay(100);
      int buttonSeconds = seconds-oldSeconds;
      //setting hours
      if (buttonSeconds > setupPeriod){
        hourSetted = true;
        changeLED(hoursLED, 0);
        hours = hoursToSet; //set the hours
        while(!digitalRead(button)){
        }
      }
    }
    //changing hours
    if (!hourSetted && buttonPressed){
      changeLED(hoursLED, 0);
      hoursToSet++;
      if (hoursToSet>11)
        hoursToSet = 0;
      hoursLED = calculateHoursLED(hoursToSet, 0);
      changeLED(hoursLED, 1);
      while(!digitalRead(button)){
      }
    }
  }

  //we do the same things for minutes programming
  boolean minutesSetted = false;
  int secondsToSet = calculateMinutesLED(seconds)*100; //calculate seconds in relation of minutesLED turned on
  int minutesLED = calculateMinutesLED(secondsToSet);
  changeLED(minutesLED, 1);
  while(!minutesSetted){
    boolean buttonPressed = false;
    oldSeconds = seconds;
    while(!digitalRead(button)){
      buttonPressed = true;
      delay(100);
      int buttonSeconds = seconds-oldSeconds;
      if (buttonSeconds > setupPeriod){
        minutesSetted = true;
        changeLED(minutesLED, 0);
        seconds = secondsToSet;
        while(!digitalRead(button)){
        }
      }
    }
    if (!minutesSetted && buttonPressed){
      changeLED(minutesLED, 0);
      secondsToSet+=100;
      if (secondsToSet>3500)
        secondsToSet = 0;
      minutesLED = calculateMinutesLED(secondsToSet);
      changeLED(minutesLED, 1);
      while(!digitalRead(button)){
      }
    }
  }

  //now we can exit from programming mode.
  //turn on all hoursLED for 1 second.
  oldSeconds = seconds;
  while(seconds-oldSeconds < 1){
    for (int i = 0; i <36; i++){
      changeLED(i, 1);
      changeLED(i, 0);
    }
  }

}



//in this funciton we pass the hours and seconds then return the number of the hours led
// that must be turn on
int calculateHoursLED(int Hours, int Seconds){
  //determine which hours LED to turn on
  int hoursLED = Hours + Hours * 2;
  if (Seconds >= 1200 && Seconds < 2400){
    hoursLED++;
  }
  else if (Seconds >= 2400 && Seconds < 3550){
    hoursLED+=2;
  }
  //if seconds is higher than 3550 we need to turn on the minutes LED 0 and, as a result, the next hours.
  else if (Seconds >= 3550) 
    //hoursLED = (hours+1) + (hours+1) * 2;
    hoursLED+=3;
  if (hoursLED > 35)
    hoursLED = 0;

  return hoursLED;
}


//in this funciton we pass the seconds and return the number of the minutes led
// that must be turn on
int calculateMinutesLED(int Seconds){
  int minutesLED = 0;
  //determine which minutes LED to blinking
  double minutesLED_count = Seconds / 100; //example: 156/100 = 1,56
  int minutesLED_count_int = minutesLED_count; // example: 1
  int minutesLED_count_dec = Seconds - (minutesLED_count_int * 100); //example: 156 - 100 = 56
  if (minutesLED_count_dec >= 50){ //if decimal is higher than 50
    minutesLED = minutesLED_count_int + 1; //turn on the next minutes LED
    if (minutesLED > 35) //if the minutesLED is higher than 35
        minutesLED = 0;  //we turn on the LED 0
  }
  else
    minutesLED = minutesLED_count_int; 

  return minutesLED;
}

//now we go sleep disabling all unnecessary peripherals
void goSleep() {
  power_timer0_disable();// disable Timer 0
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  
  //as explained in the datasheet, after and before the sleep mode, with timer 2 in asyncrhonous mode, we
  //need to write anything to this register and then wait for those registers flag go to zero.
  OCR2A = 0xFF;
  while (ASSR & (_BV(TCN2UB) | _BV(TCR2AUB) | _BV(TCR2BUB) | _BV(OCR2AUB) | _BV(OCR2BUB))) {
  }
  
  //go sleep
  sleep_mode();
  
  //the same as above
  OCR2A = 0xFF;
  while (ASSR & (_BV(TCN2UB) | _BV(TCR2AUB) | _BV(TCR2BUB) | _BV(OCR2AUB) | _BV(OCR2BUB))) {
  }
  
  power_timer0_enable();// re-enable Timer 0
}


//change LED status (on/off) to the charlieplexing matrix.
// we pass the LED number and the state (1/0 = high/low)
void changeLED(int led, boolean state) {
  int pinA, pinB, pinC;

  if (led >= 0 && led <= 5) {
    pinA = 0;
    pinB = 1;
    pinC = 2;
  }
  else if (led >= 6 && led <= 11) {
    pinA = 3;
    pinB = 4;
    pinC = 5;
    led = led - 6;
  }
  else if (led >= 12 && led <= 17) {
    pinA = 6;
    pinB = 7;
    pinC = 8;
    led = led - 12;
  }
  else if (led >= 13 && led <= 23) {
    pinA = 9;
    pinB = 10;
    pinC = 11;
    led = led - 18;
  }
  else if (led >= 24 && led <= 29) {
    pinA = 12;
    pinB = 13;
    pinC = A0;
    led = led - 24;
  }
  else if (led >= 30 && led <= 35) {
    pinA = A1;
    pinB = A2;
    pinC = A3;
    led = led - 30;
  }

  switch (led) {
  case 0:
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, INPUT);

    if (state) {
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, LOW);
    }
    else {
      digitalWrite(pinA, LOW);
      digitalWrite(pinB, LOW);
    }
    break;


  case 1:
    pinMode(pinA, INPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, OUTPUT);

    if (state) {
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, LOW);
    }
    else {
      digitalWrite(pinB, LOW);
      digitalWrite(pinC, LOW);
    }
    break;

  case 2:
    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, INPUT);

    if (state) {
      digitalWrite(pinB, HIGH);
      digitalWrite(pinA, LOW);
    }
    else {
      digitalWrite(pinB, LOW);
      digitalWrite(pinA, LOW);
    }
    break;

  case 3:
    pinMode(pinA, INPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, OUTPUT);

    if (state) {
      digitalWrite(pinC, HIGH);
      digitalWrite(pinB, LOW);
    }
    else {
      digitalWrite(pinC, LOW);
      digitalWrite(pinB, LOW);
    }
    break;

  case 4:
    pinMode(pinA, OUTPUT);
    pinMode(pinB, INPUT);
    pinMode(pinC, OUTPUT);

    if (state) {
      digitalWrite(pinA, HIGH);
      digitalWrite(pinC, LOW);
    }
    else {
      digitalWrite(pinA, LOW);
      digitalWrite(pinC, LOW);
    }
    break;

  case 5:
    pinMode(pinA, OUTPUT);
    pinMode(pinB, INPUT);
    pinMode(pinC, OUTPUT);

    if (state) {
      digitalWrite(pinC, HIGH);
      digitalWrite(pinA, LOW);
    }
    else {
      digitalWrite(pinC, LOW);
      digitalWrite(pinA, LOW);
    }
    break;

  }
}
