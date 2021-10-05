#include <Wire.h>   
#include <ATH20.h>
#include <ErriezMHZ19B.h>
#include <LiquidCrystal_I2C.h>
ATH20 ATH;
LiquidCrystal_I2C lcd(0x3F,20,4);

//I2C device at address 0x38  !  AHT20
//I2C device at address 0x3F  ! LCD

//Relay codes  
  #define ON   LOW 
  #define OFF  HIGH
  #define Hum  8   
  #define Exh  10 
  #define Fans 9

//MH-Z19

  // Pin defines
  #if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_SAM_DUE)
    #define mhzSerial           Serial1 // Use second hardware serial

  #elif defined(ARDUINO_ARCH_AVR)
    #define MHZ19B_TX_PIN        4
    #define MHZ19B_RX_PIN        5

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
  #elif defined(ARDUINO_ARCH_ESP8266)
    #define MHZ19B_TX_PIN        D5
    #define MHZ19B_RX_PIN        D6

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
  #elif defined(ARDUINO_ARCH_ESP32)
    #define MHZ19B_TX_PIN        18
    #define MHZ19B_RX_PIN        19

    #include <SoftwareSerial.h>          // Use software serial
    SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);
  #else
    #error "May work, but not tested on this target"
  #endif

ErriezMHZ19B mhz19b(&mhzSerial);
float humi, temp;
int16_t ppm;


//Exhaust timer controls
  unsigned long ExhaustOff = 600000;
  unsigned long ExhaustOn  = 800000;
  unsigned long PrevExh = 0;
  unsigned long PrevLCDReset = 0;
  unsigned long getDataTimer = 0;

  byte ExhaustState;
  byte HumState; 
  char LcdOn='I';
  char LcdOff=' ';

void printErrorCode(int16_t ppm)
  {
    // Print error code
    switch (ppm) {
        case MHZ19B_RESULT_ERR_CRC:
            Serial.println(F("CRC error"));
            break;
        case MHZ19B_RESULT_ERR_TIMEOUT:
            Serial.println(F("RX timeout"));
            break;
        default:
            Serial.print(F("Error: "));
            Serial.println(ppm);
            break;
    }
  }


void setup() {
  Wire.begin();
  lcd.init();
  lcd.setBacklight(HIGH);
  Serial.begin(115200);
  char firmwareVersion[5];
  ATH.begin();

//MH-Z19    
    mhzSerial.begin(9600);
    mhz19b.getVersion(firmwareVersion, sizeof(firmwareVersion));
    Serial.print(mhz19b.getRange());
    mhz19b.setAutoCalibration(true);

pinMode(Hum,OUTPUT); 
pinMode(Exh,OUTPUT);  
pinMode(Fans,OUTPUT);  
digitalWrite(Hum,OFF);  
digitalWrite(Exh,OFF); 
digitalWrite(Fans,OFF);
}

void loop() {
//LCD and Sensors
  if (millis() < 5000){
    lcd.init();
    lcd.setBacklight(HIGH);
  };
  if (millis() - getDataTimer >= 2000){
    ppm = mhz19b.readCO2();
    int ret = ATH.getSensor(&humi, &temp);
    lcd.setCursor(0,0);
    lcd.print("CO2:      ppm");
    lcd.setCursor(5,0);
    lcd.print(ppm);
    lcd.setCursor(0,1);
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print(" C");
    lcd.setCursor(0,2);
    lcd.print("Hum: ");
    lcd.print(humi*100);
    lcd.print("%");
    lcd.setCursor(0,3);  
    lcd.print("Fans: ");  
    getDataTimer = millis();
  };

//Serial 
   Serial.print("CO2: ");
   Serial.print(ppm);;
   Serial.println("ppm   ");
   Serial.print("Temp: ");
   Serial.print(temp);
   Serial.println(" C");
   Serial.print("Hum: ");
   Serial.print(humi*100);
   Serial.println("%");
//Controls
//Humidity
    if (humi < 0.80 && ExhaustState == LOW) { 
    HumState = HIGH;  
    digitalWrite(Hum,ON);
    digitalWrite(Fans,ON);
    lcd.setCursor(18,2);  
    lcd.write(LcdOn);
    lcd.setCursor(18,3);
    lcd.write(LcdOn);}

    if (humi > 0.95 && ExhaustState == LOW){
    HumState = LOW;
    digitalWrite(Hum,OFF);
    digitalWrite(Fans,OFF);
    lcd.setCursor(18,2);
    lcd.print(LcdOff); 
    lcd.setCursor(18,3);
    lcd.write(LcdOff);}

//Exhaust
    if (HumState == LOW && ExhaustState == LOW && millis() - PrevExh > ExhaustOff) {
    ExhaustState = HIGH;
    PrevExh = millis();
    digitalWrite(Exh,ON);
    digitalWrite(Fans,ON);
    lcd.setCursor(18,0);  
    lcd.write(LcdOn);
    lcd.setCursor(18,3);
    lcd.write(LcdOn);}

    if (HumState == LOW && ExhaustState == HIGH && millis() - PrevExh > ExhaustOn) { 
    ExhaustState = LOW;
    PrevExh = millis();
    digitalWrite(Exh,OFF);
    digitalWrite(Fans,OFF);
    lcd.setCursor(18,0);
    lcd.write(LcdOff);
    lcd.setCursor(18,3);
    lcd.write(LcdOff);}

//LCD Reset
    if (millis() - PrevLCDReset > 30000){
    lcd.init();
    PrevLCDReset = millis();
    }
}
