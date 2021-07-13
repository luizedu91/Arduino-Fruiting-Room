//Set
  #include "DHT.h" 
  #include "Wire.h"
  #include "Adafruit_LiquidCrystal.h"
  #include "MQ135.h"
  #define DHTPIN 6    
  #define DHTTYPE DHT22  
  #define PIN_MQ135 A0
  Adafruit_LiquidCrystal lcd(8, 7, 5, 4, 3, 2);  
  DHT dht(DHTPIN, DHTTYPE);
  MQ135 gasSensor = MQ135(A0);
  float temperature; 
  float humidity;

  //Exhaust timer controls
  unsigned long ExhaustOff = 600000;
  unsigned long ExhaustOn  = 600000;
  unsigned long Now = 0;
  unsigned long PrevExh = 0;
  unsigned long LCDReset = 0; 
  unsigned long PrevLCDReset = 0;
  byte ExhaustState = LOW;
  byte HumState = LOW;
  //Relay codes  
  #define ON   LOW 
  #define OFF  HIGH
  #define Hum  9   
  #define Exh  10 
  #define Fans 11

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.begin(20, 4);

  pinMode(Hum,OUTPUT); 
  pinMode(Exh,OUTPUT);  
  pinMode(Fans,OUTPUT);  
  digitalWrite(Hum,OFF);  
  digitalWrite(Exh,OFF); 
  digitalWrite(Fans,OFF);
  }

void loop() {
  LCDReset = millis();
  Now = millis();

  Serial.print("Now: ");
  Serial.println(Now);
  Serial.print("PrevExh: ");
  Serial.println(PrevExh);
  Serial.print("ExhaustState: ");
  Serial.println(ExhaustState);
  Serial.println();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float rzero = gasSensor.getRZero();
  float ppm = gasSensor.getPPM();

  lcd.setCursor(0,0);
  lcd.print("CO2: ");
  lcd.print(ppm);
  lcd.print("ppm   ");
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");
  lcd.setCursor(0,2);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(0,3);  
  lcd.print("Fans: ");
  delay(1500);

  //Controls
  //Humidity
  if (humidity < 86 && ExhaustState == LOW) { 
    HumState = HIGH;  
    digitalWrite(Hum,ON);
    digitalWrite(Fans,ON);
    lcd.setCursor(18,2);  
    lcd.write("O");
    lcd.setCursor(18,3);
    lcd.write("O");}

  if (humidity > 98 && ExhaustState == LOW){
    HumState = LOW;
    digitalWrite(Hum,OFF);
    digitalWrite(Fans,OFF);
    lcd.setCursor(18,2);
    lcd.print("  "); 
    lcd.setCursor(18,3);
    lcd.write("  ");}

  //Exhaust
  if (HumState == LOW && ExhaustState == LOW && Now - PrevExh > ExhaustOff) {
  ExhaustState = HIGH;   
  PrevExh = Now;
  digitalWrite(Exh,ON);
  digitalWrite(Fans,ON);
  lcd.setCursor(18,0);  
  lcd.write("O");
  lcd.setCursor(18,3);
  lcd.write("O");}

  if (HumState == LOW && ExhaustState == HIGH && Now - PrevExh > ExhaustOn) { 
  ExhaustState = LOW;
  PrevExh = Now;
  digitalWrite(Exh,OFF);
  digitalWrite(Fans,OFF);
  lcd.setCursor(18,0);
  lcd.write("  ");
  lcd.setCursor(18,3);
  lcd.write("  ");}
  //LCD Reset
  if (LCDReset - PrevLCDReset >= 30000000){
  lcd.begin(20, 4);
  PrevLCDReset = Now; }} 