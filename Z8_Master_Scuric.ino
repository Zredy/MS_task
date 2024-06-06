/*
 *  ZADATAK: Programirati master preko RS485 protokola, po predefiniranim kodovima i tipu
 *  
 *  IZRADIO:  Alen Šćuric
 *    DATUM:  14.05.2024.
 *    
 *  SPOJEVI:  Port D RS485 komunikacija, LCD na Port B i tipkala na Port C
 *
 *  CARRIER: # 07 05 AA /
 *  RECEIVE: * 07 05 AA /
 *  07 - ID primatelja/slave-a
 *  05 - Naredba za slave, podatak od slave-a
 *  AA - cheksum, 7+5+AA = 0
 *
 */

#include <FSBAVR.h>
#include<TimerOne.h>   //dodano
#include<LiquidCrystal.h> //dodano
#include<OneWire.h>     //dodano
byte CTRL = PORT_D4;
#define TRANSMIT PORTD |= (1 << 4)
#define RECEIVE PORTD &= ~(1 << 4)

#define ID "02"

int StanjeH3ID2 = -1;
int StanjeH4ID2 = -1;
int StanjeH5ID2 = -1;
int StanjeH6ID2 = -1;
int StanjeH3ID3 = -1;
int StanjeH4ID3 = -1;
int StanjeH5ID3 = -1;
int StanjeH6ID3 = -1;

int TipkaloDelay = 250;


String ByteToHex(byte by_in){
  String val = String(by_in, HEX);
  if(val.length() == 1){
    val = "0" + val;
  }
  val.toUpperCase();
  return val; 
}

String HexToByte(String str_in){
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < str_in.length(); i++) {
    
    nextInt = int(str_in.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  String val = String(decValue);
  if (val.length() == 1){
    val = "0" + val;
  }
  return val;
}

//inicijalizacija LCD ekrana DODANO
const int rs=51, en=50, b4=10, b5=11, b6=12, b7=13;
LiquidCrystal lcd(rs, en, b4, b5, b6, b7);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  //pokretanje LCD-a
  lcd.begin(20,4);
  
  //dodano za timer
  Timer1.initialize(333333); //frekvencija od 3Hz
  Timer1.attachInterrupt( stanje );   //spajamo ga na funkciju stanje koja printa na LCD temp., i stanje porta B slejvova

  
  
  //dodano za tipkala
  pinMode(37,INPUT_PULLUP); //pali/gasi H3 slave ID=2 
  pinMode(36,INPUT_PULLUP); //pali/gasi H4 slave ID=2 
  pinMode(35,INPUT_PULLUP); //pali/gasi H5 slave ID=2 
  pinMode(34,INPUT_PULLUP); //pali/gasi H6 slave ID=2 
  pinMode(33,INPUT_PULLUP); //pali/gasi H3 slave ID=3 
  pinMode(32,INPUT_PULLUP); //pali/gasi H4 slave ID=3
  pinMode(31,INPUT_PULLUP); //pali/gasi H5 slave ID=3 
  pinMode(30,INPUT_PULLUP); //pali/gasi H6 slave ID=3 
 
}

String calc_checksum(byte slave, byte command){
  byte val = 00;
  val = 256 - slave - HexToByte(String(command)).toInt();
  return ByteToHex(val);
}

void transmit_data(byte slave, byte command){
  String data = "";
  data += "#";
  data += ByteToHex(slave);
  if(command < 10) data += "0";
  data += command;
  data += calc_checksum(slave, command);
  data += "/";
  Serial.println(data);
}

void stanje(){
  
  
  // prvi red
  lcd.setCursor(0,0);
  lcd.print("StanjePBS2: ");
  lcd.setCursor(12,0);
  //lcd.print(to kaj reciver da sa S2);
  //drugi red
  lcd.setCursor(0,1);
  lcd.print("StanjePBS3: ");
  lcd.setCursor(12,1);
  //lcd.print(to kaj reciver da sa S3);
  
  
  //treći red
  lcd.setCursor(0,2);
  lcd.print("Temp S2=");
  lcd.setCursor(9,2);
  //lcd.print(tempS2);
 
  
  // četvrti red
  lcd.setCursor(0,3);
  lcd.print("Temp S3=");
  lcd.setCursor(9,3);
  //lcd.print(tempS3);

}

void loop() {

  if(digitalRead(37) == 0)
  {
    StanjeH3ID2 *= -1;
    if (StanjeH3ID2 > 0) transmit_data(02,31); //Pali H3 ID2
    else transmit_data(02,30); //Gasi H3 ID2
    delay(TipkaloDelay);
  }
  
  if(digitalRead(36) == 0)
  {
    StanjeH4ID2 *= -1;
    if (StanjeH4ID2 > 0) transmit_data(02,41); //Pali H4 ID2
    else transmit_data(02,40); //Gasi H4 ID2
    delay(TipkaloDelay);
  }
  
  if(digitalRead(35) == 0)
  {
    StanjeH5ID2 *= -1;
    if (StanjeH5ID2 > 0) transmit_data(02,51); //Pali H5 ID2
    else transmit_data(02,50); // Gasi H5 ID2
    delay(TipkaloDelay);
  }
  
  if(digitalRead(34) == 0)
  {
    StanjeH6ID2 *= -1;
    if (StanjeH6ID2 > 0) transmit_data(02,61); //Pali H6 ID2
    else transmit_data(02,60); //Gasu H6 ID2
    delay(TipkaloDelay);
  }
  
  if(digitalRead(33) == 0)
  {
    StanjeH3ID3 *= -1;
    if (StanjeH3ID3 > 0) transmit_data(03,31); // Pali H3 ID3
    else transmit_data(03,30); //Gasi H3 ID3
    delay(TipkaloDelay);
  }
  
  if(digitalRead(32) == 0)
  {
    StanjeH4ID3 *= -1;
    if (StanjeH4ID3 > 0) transmit_data(03,41); // Pali H4 ID3
    else transmit_data(03,40); // Gasi H4 ID3
    delay(TipkaloDelay);
  }
  
  if(digitalRead(31) == 0)
  {
    StanjeH5ID3 *= -1;
    if (StanjeH5ID3 > 0) transmit_data(03,51); //Pali H5 ID3
    else transmit_data(03,50); //Gasi H5 ID3
    delay(TipkaloDelay);
  }
  
  if(digitalRead(30) == 0)
  {
    StanjeH6ID3 *= -1;
    if (StanjeH6ID3 > 0) transmit_data(03,61); //Pali H6 ID3
    else transmit_data(03,60); //Gasi H6 ID3
    delay(TipkaloDelay);
  }

  
  delay(1000);
}
