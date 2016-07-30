//============================================================================
// Name        : FreecomRF43.ino
// Author      : R. BELLO <https://github.com/rbello>
// Version     : 1.0
// Copyright   : Creative Commons (by)
// Description : A small physical tool that displays the radio messages sent by home automation components using RF 433Mhz.
//============================================================================

#include <LiquidCrystal.h>
#include <RCSwitch.h>

#define INT_433_SENSOR  0     // Interruption pin for RF 433 sensor (#2)
#define PIN_433_EMITTER 11    // Pin for RF 433 emitter
#define PIN_LCD_RS 10         // Pin for LCD display
#define PIN_LCD_E  9          // Pin for LCD display
#define PIN_LCD_D4 8          // Pin for LCD display
#define PIN_LCD_D5 7          // Pin for LCD display
#define PIN_LCD_D6 6          // Pin for LCD display
#define PIN_LCD_D7 5          // Pin for LCD display
#define PIN_TILT 4            // Pin for tilt switch
#define PIN_LED  3            // Pin for external LED

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
  lcd.setCursor(0, 0);
  lcd.print("  Listening...");
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_LED, LOW);
  delay(100);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_LED, LOW);
  delay(100);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_LED, LOW);
  Serial.println("Ready");

}

void sendRf(long data) {
  // LED on
  digitalWrite(PIN_LED, HIGH);
  // Log
  Serial.print("Rf433Mhz: sent ");
  Serial.println(data);
  // LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending...");
  lcd.setCursor(0, 1);
  char buf[50];
  sprintf(buf, "%lu", data);
  lcd.print(buf);
  // Rf433 send
  rf433write.send(data, 32);
  rf433write.send(data, 32);
  // LED off
  digitalWrite(PIN_LED, LOW);
}

int tilt_hits = 0;
unsigned long tilt_time = 0;
bool tilt_state = false;
long last_value = 0;

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
        if (tilt_hits >= 5) {
          Serial.println("Tilt: hit");
          if (last_value > 0) {
            sendRf(last_value);
          }
          else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("  Listening...");
          }
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
        // Save value
        last_value = value;
        // LCD display
        lcd.clear();
        char buf2[50];
        sprintf(buf2, "Rcvd: P=%d L=%d", rf433read.getReceivedProtocol(), rf433read.getReceivedBitlength());
        lcd.print(buf2);
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
        sendRf(data);
      }
    }
}
