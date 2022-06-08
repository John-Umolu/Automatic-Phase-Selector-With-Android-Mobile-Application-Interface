#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetooth(11,12);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

int phase1Switch = 10;
int phase2Switch = 9;
int phase3Switch = 8;
//ZMPT101B
double sensorValue1 = 0;
double sensorValue2 = 0;
double sensorValue3 = 0;
int val1[100];
int val2[100];
int val3[100];
int max_v1 = 0;
int max_v2 = 0;
int max_v3 = 0;
double VmaxD1 = 0;
double VmaxD2 = 0;
double VmaxD3 = 0;
double VeffD1 = 0;
double VeffD2 = 0;
double VeffD3 = 0;
double Veff1 = 0;
double Veff2 = 0;
double Veff3 = 0;
//
float phase1Vin = 0.0;
float phase2Vin = 0.0;
float phase3Vin = 0.0;
float phase1Prev = 0.0;
float phase2Prev = 0.0;
float phase3Prev = 0.0;
//
int phase1Count = 0;
int phase2Count = 0;
int phase3Count = 0;
//
int manual = 0;
//
unsigned char sample_count = 0;
//
void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  lcd.begin(20,4);
  pinMode(phase1Switch, OUTPUT);
  pinMode(phase2Switch, OUTPUT);
  pinMode(phase3Switch, OUTPUT);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("      AUTOMATIC     ");
  lcd.setCursor(0,1);
  lcd.print("     THREE PHASE    ");
  lcd.setCursor(0,2);
  lcd.print("       SELECTOR     ");
  lcd.setCursor(0,3);
  lcd.print("        SYSTEM      ");
  delay(2000);
  manual = EEPROM.read(0);
  if (manual > 3){
    manual = 0;
    EEPROM.write(0,0);
  }
  digitalWrite(phase1Switch, HIGH);
  digitalWrite(phase2Switch, HIGH);
  digitalWrite(phase3Switch, HIGH);
}

void loop() {
  if(bluetooth.available() > 0){
    String readtext = "";
    while(bluetooth.available())
    {
      char CharRead = bluetooth.read();
      readtext += CharRead; 
    }
    delay(10);
    readtext.toUpperCase();
    Serial.println("Command: " + readtext);
    if (readtext.indexOf("C") >= 0){
      EEPROM.write(0,1);
      manual = 1;
    }
    if (readtext.indexOf("D") >= 0){
      EEPROM.write(0,2);
      manual = 2;
    }
    if (readtext.indexOf("E") >= 0){
      EEPROM.write(0,3);
      manual = 3;
    }
    if (readtext.indexOf("F") >= 0){
      manual = 0;
      EEPROM.write(0,0);
    }
  }
  else{
    // For zmpt101b voltage sensor module
    for ( int i = 0; i < 100; i++ ) {
      sensorValue1 = analogRead(A0);
      sensorValue2 = analogRead(A1);
      sensorValue3 = analogRead(A2);
      //
      if (analogRead(A0) > 511) {
        val1[i] = sensorValue1;
      }
      if (analogRead(A1) > 511) {
        val2[i] = sensorValue2;
      }
      if (analogRead(A2) > 511) {
        val3[i] = sensorValue3;
      }
      //
      if (analogRead(A0) <= 511) {
        val1[i] = 0;
      }
      if (analogRead(A1) <= 511) {
        val2[i] = 0;
      }
      if (analogRead(A2) <= 511) {
        val3[i] = 0;
      }
      delay(1);
    } 
    max_v1 = 0;
    max_v2 = 0;
    max_v3 = 0;
    //
    for ( int i = 0; i < 100; i++ )
    {
      if (val1[i] > max_v1)
      {
        max_v1 = val1[i];
      }
      if (val2[i] > max_v2)
      {
        max_v2 = val2[i];
      }
      if (val3[i] > max_v3)
      {
        max_v3 = val3[i];
      }
      val1[i] = 0;
      val2[i] = 0;
      val3[i] = 0;
    }
    //
    if (max_v1 != 0) {
      VmaxD1 = max_v1;
      VeffD1 = VmaxD1 / sqrt(2);
      Veff1 = (((VeffD1 - 420.76) / -90.24) * -210.2) + 210.2;
    }
    if (max_v2 != 0) {
      VmaxD2 = max_v2;
      VeffD2 = VmaxD2 / sqrt(2);
      Veff2 = (((VeffD2 - 420.76) / -90.24) * -210.2) + 210.2;
    }
    if (max_v3 != 0) {
      VmaxD3 = max_v3;
      VeffD3 = VmaxD3 / sqrt(2);
      Veff3 = (((VeffD3 - 420.76) / -90.24) * -210.2) + 210.2;
    }
    //
    if (max_v1 == 0) {
      Veff1 = 0;
    }
    if (max_v2 == 0) {
      Veff2 = 0;
    }
    if (max_v3 == 0) {
      Veff3 = 0;
    }
    //
    VmaxD1 = 0;
    VmaxD2 = 0;
    VmaxD3 = 0;
    //
    phase1Vin = Veff1;
    phase2Vin = Veff2;
    phase3Vin = Veff3;
    //
    if(phase1Vin < 0){
      phase1Vin = 0;
    }
    if(phase2Vin < 0){
      phase2Vin = 0;
    }
    if(phase3Vin < 0){
      phase3Vin = 0;
    }
    //
    Serial.println("Phase1: " + String(phase1Vin) + ", Phase2: " + String(phase2Vin) + ", Phase3: " + String(phase3Vin));
    if(phase1Vin <= 180 && manual == 0){
      phase1Vin = 0;
    }
    if(phase2Vin <= 180 && manual == 0){
      phase2Vin = 0;
    }
    if(phase3Vin <= 180 && manual == 0){
      phase3Vin = 0;
    }
    //
    if(manual == 1){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("MANUAL MODE IN USE");
      delay(500);
      digitalWrite(phase1Switch, LOW);
      digitalWrite(phase2Switch, HIGH);
      digitalWrite(phase3Switch, HIGH);
    }
    if(manual == 2){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("MANUAL MODE IN USE");
      delay(500);
      digitalWrite(phase1Switch, HIGH);
      digitalWrite(phase2Switch, LOW);
      digitalWrite(phase3Switch, HIGH);
    }
    if(manual == 3){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("MANUAL MODE IN USE");
      delay(500);
      digitalWrite(phase1Switch, HIGH);
      digitalWrite(phase2Switch, HIGH);
      digitalWrite(phase3Switch, LOW);
    }
    if(phase1Vin > 270 && phase2Vin > 270 && phase3Vin > 270){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("STATUS: HIGH INPUT  ");
      digitalWrite(phase1Switch, HIGH);
      digitalWrite(phase2Switch, HIGH);
      digitalWrite(phase3Switch, HIGH);
    }
    if(phase1Vin > 270){
      digitalWrite(phase1Switch, HIGH);
    }
    if(phase2Vin > 270){
      digitalWrite(phase2Switch, HIGH);
    }
    if(phase3Vin > 270){
      digitalWrite(phase3Switch, HIGH);
    }
    if(phase1Vin == 0 && phase2Vin == 0 && phase3Vin == 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("STATUS: NO INPUT    ");
    }
    //
    if(phase1Vin >= phase2Vin && phase1Vin >= phase3Vin && phase1Vin > 150 && phase1Vin <= 270 && manual == 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("IN USE: RED PHASE");
      delay(500);
      digitalWrite(phase1Switch, LOW);
      digitalWrite(phase2Switch, HIGH);
      digitalWrite(phase3Switch, HIGH);
    }
    if(phase2Vin >= phase1Vin && phase2Vin >= phase3Vin && phase2Vin > 150 && phase2Vin <= 270 && manual == 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("IN USE: YELLOW PHASE");
      delay(500);
      digitalWrite(phase1Switch, HIGH);
      digitalWrite(phase2Switch, LOW);
      digitalWrite(phase3Switch, HIGH);
    }
    if(phase3Vin >= phase1Vin && phase3Vin >= phase2Vin && phase3Vin > 150 && phase3Vin <= 270 && manual == 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RED PHASE: " + String(phase1Vin,1) + "V ");
      lcd.setCursor(0,1);
      lcd.print("YELLOW PHASE: " + String(phase2Vin,1) + "V ");
      lcd.setCursor(0,2);
      lcd.print("BLUE PHASE: " + String(phase3Vin,1) + "V ");
      lcd.setCursor(0,3);
      lcd.print("IN USE: BLUE PHASE");
      delay(500);
      digitalWrite(phase1Switch, HIGH);
      digitalWrite(phase2Switch, HIGH);
      digitalWrite(phase3Switch, LOW);
    }
    //
    if(phase1Vin > 270){
      digitalWrite(phase1Switch, HIGH);
    }
    if(phase2Vin > 270){
      digitalWrite(phase2Switch, HIGH);
    }
    if(phase3Vin > 270){
      digitalWrite(phase3Switch, HIGH);
    }
    if(phase1Vin > 270){
      digitalWrite(phase1Switch, HIGH);
    }
    if(phase2Vin > 270){
      digitalWrite(phase2Switch, HIGH);
    }
    if(phase3Vin > 270){
      digitalWrite(phase3Switch, HIGH);
    }
    bluetooth.println("G" + String(phase1Vin) + "H" + String(phase2Vin) + "I" + String(phase3Vin));
    delay(1500);
  }
}
