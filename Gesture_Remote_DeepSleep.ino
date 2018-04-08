#include <avr/sleep.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <IRremote.h>
IRsend irsend;
int wakePin = 2;                 // pin used for waking up
int sleepStatus = 0;             // variable to store a request for sleep
int count = 0;                   // counter
#define APDS9960_INT    2 // Needs to be an interrupt pin 
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;


long time0 = millis();


//LG volUp IR code
unsigned int  volUp[67] = {8950, 4400, 600, 500, 600, 550, 550, 1650, 550, 550, 600, 500, 600, 550, 550, 550, 600, 500, 600, 1600, 600, 1650, 600, 500, 600, 1600, 600, 1650, 550, 1650, 600, 1600, 600, 1650, 600, 500, 600, 1600, 600, 550, 550, 550, 600, 500, 600, 550, 550, 550, 550, 550, 550, 1650, 600, 550, 550, 1650, 600, 1600, 600, 1650, 550, 1650, 600, 1600, 600, 1650, 600}; // NEC 20DF40BF


//LG volDown IR code
unsigned int  volDown[67] = {8950, 4400, 600, 550, 550, 550, 550, 1650, 600, 500, 600, 550, 550, 550, 550, 550, 600, 500, 600, 1650, 550, 1650, 600, 500, 600, 1650, 550, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 1600, 600, 1650, 600, 500, 600, 550, 550, 500, 600, 550, 600, 500, 600, 500, 600, 550, 550, 550, 550, 1650, 600, 1650, 550, 1650, 550, 1650, 600, 1650, 600, 1600, 600}; // NEC 20DFC03F


//LG chDown  IR code
unsigned int  chDown[67] = {8950, 4400, 600, 550, 550, 550, 600, 1600, 600, 550, 550, 550, 550, 550, 550, 550, 600, 500, 600, 1650, 550, 1650, 550, 550, 600, 1650, 550, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 1650, 550, 550, 600, 500, 600, 550, 550, 550, 550, 550, 550, 550, 600, 500, 600, 550, 550, 1650, 550, 1650, 600, 1650, 550, 1650, 550, 1650, 600, 1650, 550, 1650, 600}; // NEC 20DF807F


//LG chUp  IR code
unsigned int  chUp[67] = {8900, 4450, 550, 550, 550, 550, 600, 1650, 550, 550, 600, 500, 550, 550, 600, 500, 600, 550, 550, 1700, 500, 1650, 600, 550, 550, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 1600, 600, 550, 550, 550, 600, 550, 550, 550, 550, 550, 550, 550, 550, 550, 600, 550, 550, 1650, 550, 1650, 600, 1650, 550, 1650, 550, 1650, 600, 1650, 600, 1600, 600, 1600, 600}; // NEC 20DF00FF


//LG power  IR code
unsigned int  power [67] = {8900, 4450, 550, 550, 550, 550, 600, 1650, 550, 550, 600, 500, 550, 550, 600, 550, 550, 550, 550, 1650, 550, 1650, 600, 550, 550, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 1600, 600, 550, 550, 550, 600, 550, 550, 1600, 600, 550, 550, 550, 600, 500, 600, 550, 550, 1650, 550, 1650, 600, 1650, 600, 500, 550, 1650, 600, 1650, 550, 1650, 600, 1600, 600}; // NEC 20DF10EF

//LG mute IR code
unsigned int  mute[67] = {8900, 4450, 600, 500, 550, 550, 600, 1650, 550, 550, 550, 550, 550, 550, 600, 550, 550, 550, 550, 1650, 600, 1600, 600, 550, 550, 1650, 600, 1600, 600, 1650, 600, 1600, 600, 1600, 600, 1650, 600, 500, 600, 550, 550, 1650, 550, 550, 600, 500, 600, 550, 550, 550, 550, 550, 550, 1650, 600, 1650, 550, 550, 550, 1650, 600, 1650, 550, 1650, 600, 1600, 600}; // NEC 20DF906F

// 38kHz carrier frequency for the NEC protocol
int khz = 38;




void wakeUpNow()        // here the interrupt is handled after wakeup
{
  // execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  // we don't really need to execute any special functions here, since we
  // just want the thing to wake up
}

void setup()
{
  pinMode(wakePin, INPUT_PULLUP);
  pinMode(APDS9960_INT, INPUT_PULLUP);
  // Initialize Serial port
  Serial.begin(9600);


  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }


  attachInterrupt(0, wakeUpNow, HIGH ); // use interrupt 0 (pin 2) and run function
  // wakeUpNow when pin 2 gets LOW
}

void sleepNow()         // here we put the arduino to sleep
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

  sleep_enable();          // enables the sleep bit in the mcucr register
  // so sleep is possible. just a safety pin

  attachInterrupt(0, interruptRoutine, HIGH);
  // wakeUpNow when pin 2 gets LOW

  sleep_mode();            // here the device is actually put to sleep!!
  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP

  sleep_disable();         // first thing after waking from sleep:
  // disable sleep...

}

void loop()
{
  if ( isr_flag == 1 ) {
    detachInterrupt(0);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(0, interruptRoutine, HIGH);
  }


  delay(100);
  sleepNow();     // sleep function called here

}

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
  if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        irsend.sendRaw(volUp, sizeof(volUp) / sizeof(volUp[0]), khz); //Note the approach used to automatically calculate the size of the array.
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        irsend.sendRaw(volDown, sizeof(volDown) / sizeof(volDown[0]), khz); //Note the approach used to automatically calculate the size of the array.
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        irsend.sendRaw(chDown, sizeof(chDown) / sizeof(chDown[0]), khz); //Note the approach used to automatically calculate the size of the array.
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        irsend.sendRaw(chUp, sizeof(chUp) / sizeof(chUp[0]), khz); //Note the approach used to automatically calculate the size of the array.
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        irsend.sendRaw(mute, sizeof(mute) / sizeof(mute[0]), khz); //Note the approach used to automatically calculate the size of the array.
        break;
      case DIR_FAR:
        Serial.println("FAR");
        irsend.sendRaw(power, sizeof(power) / sizeof(power[0]), khz); //Note the approach used to automatically calculate the size of the array.
        break;
      default:
        Serial.println("NONE");
    }
  }
}
