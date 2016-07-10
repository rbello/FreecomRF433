#include <LiquidCrystal.h>
#include <RCSwitch.h>

#define INT_433_SENSOR 0 // Interruption pin for RF 433 sensor (#2)
#define PIN_433_EMITTER 3 // Pin for RF 433 emitter
#define PIN_LCD_RS 4 // Pin for LCD display
#define PIN_LCD_E  5 // Pin for LCD display
#define PIN_LCD_D4 6 // Pin for LCD display
#define PIN_LCD_D5 7 // Pin for LCD display
#define PIN_LCD_D6 8 // Pin for LCD display
#define PIN_LCD_D7 9 // Pin for LCD display
#define PIN_TILT 10 // Pin for tilt switch
#define PIN_LED 11  // Pin for external LED

// LCD display
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// Sensors RF 433Mhz
RCSwitch rf433read = RCSwitch();
RCSwitch rf433write = RCSwitch();

void setup() {

  // Status LED
  pinMode(PIN_LED, OUTPUT);

  // Tilt switch
  pinMode(PIN_TILT, INPUT);

  // Init serial
  Serial.begin(9600);

  // Init RF 433
  rf433read.enableReceive(INT_433_SENSOR);
  rf433read.setPulseLength(321);
  rf433write.enableTransmit(PIN_433_EMITTER);
  rf433write.setPulseLength(320);
  rf433write.setProtocol(2);

  // Init LCD display
  lcd.begin(16, 2);

  // Display welcome message
  lcd.clear();
  lcd.print("Welcome");

}

int tilt_hits = 0;
unsigned long tilt_time = 0;
bool tilt_state = false;

void loop() {

    // Tilt switch
    if (digitalRead(PIN_TILT) == HIGH) {
      if (!tilt_state) {
        tilt_state = true;
        tilt_hits++;
      }
    }
    else {
      tilt_state = false;
    }
    if (millis() - tilt_time > 500) {
        tilt_time = millis();
        if (tilt_hits >= 2) {
          Serial.println("Tilt: hit");
          lcd.clear();
        }
        tilt_hits = 0;
    }

    // RF 433 receiver
    if (rf433read.available())
    {
      unsigned long value = rf433read.getReceivedValue();
      if (value == 0)
      {
        Serial.println("Rf433Mhz: rcvd error");
      }
      else {
        // LED on
        digitalWrite(PIN_LED, HIGH);
        // Log
        Serial.print("Rf433Mhz: rcvd ");
        char buf[50];
        sprintf(buf, "%lu", value);
        Serial.println(buf);
        // LCD display
        lcd.clear();
        lcd.print("Received");
        lcd.setCursor(0, 1);
        lcd.print(buf);
        // LED off
        digitalWrite(PIN_LED, LOW);
      }
      rf433read.resetAvailable();
    }

    // Handle inputs from serial
    if (Serial.available() > 0)
    {
      long data = String(Serial.readString()).toInt();
      if (data > 0)
      {
        // LED on
        digitalWrite(PIN_LED, HIGH);
        // Log
        Serial.print("Rf433Mhz: sent ");
        Serial.println(data);
        // Rf433 se
        rf433write.send(data, 32);
        rf433write.send(data, 32);
        // LED off
        digitalWrite(PIN_LED, LOW);
      }
    }
}