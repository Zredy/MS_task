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
byte CTRL = PORT_D4;
#define TRANSMIT PORTD |= (1 << 4)
#define RECEIVE PORTD &= ~(1 << 4)

#define ID "02"

#define timeout 10

volatile int stanja[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

unsigned long readStartTime = 0;
volatile int dataDrops = 0;

volatile byte send_slave = 02;
volatile byte send_comm = 20;

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
  Serial.begin(115200);
  Serial.setTimeout(10);
  
  //pokretanje LCD-a
  lcd.begin(20,4);
  
  //dodano za timer
  Timer1.initialize(166666); //frekvencija od 3Hz
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

String receive_data(byte slave){
  sei();
  readStartTime = millis();
  String input = "";
  String rec_data = "";
  if(Serial.available() > 0){
    if((millis() - readStartTime) > timeout) {
      dataDrops++;
      return "";
    }
    input = Serial.readStringUntil("/");
    Serial.println(input);
  }
  
  if(input.charAt(0) != '*') return "";
  if(!input.substring(1,3).equals(ByteToHex(slave))) return "";
  rec_data = HexToByte(input.substring(3,5));
  String cs = input.substring(5,7);
  return(rec_data);
  
  cli();
}

// * 02 AA BB /
void stanje(){
  String temp = "";
  transmit_data(send_slave, send_comm);

  temp = receive_data(send_slave);
  //Serial.println(temp);

  if(!temp.equals("")){
    if(send_comm == 20){
      lcd.setCursor(12,send_slave-2);
      lcd.print("00000000");
      lcd.setCursor(20-String(temp.toInt(),BIN).length() ,send_slave-2);
      lcd.print(String(temp.toInt(),BIN));
    }
    if(send_comm == 25){
      lcd.setCursor(9,send_slave);
      lcd.print(temp);
    }
    if(send_comm == 20) send_comm = 25;
    else if(send_comm == 25) send_comm = 20;
  }
  

  // prvi red
  lcd.setCursor(0,0);
  lcd.print("StanjePBS2: ");
  //lcd.setCursor(12,0);
  //lcd.print("00000000");
  //lcd.setCursor(20-String(test.toInt(),BIN).length() ,0);
  //drugi red
  lcd.setCursor(0,1);
  lcd.print("StanjePBS3: ");
  //lcd.setCursor(12,1);
  //lcd.print("00000000");
 // lcd.setCursor(20-String(test.toInt(),BIN).length() ,0);

  //treći red
  lcd.setCursor(0,2);
  lcd.print("Temp S2=");
  //lcd.setCursor(9,2);
  //lcd.print(tempS2);
  lcd.setCursor(12,2);
  lcd.print("X: ");
  lcd.print(dataDrops);
  
  // četvrti red
  lcd.setCursor(0,3);
  lcd.print("Temp S3=");
  //lcd.setCursor(9,3);
  //lcd.print(tempS3);

}

void loop() {
  ProvjeriTipke();
  delay(100);  
}

void ProvjeriTipke()
{
   if(digitalRead(37) == 0)
  {
    stanja[0] *= -1;
    if (stanja[0] > 0) transmit_data(02,31); //Pali H3 ID2
    else transmit_data(02,30); //Gasi H3 ID2
    while(digitalRead(37) == 0);
  }
  
  if(digitalRead(36) == 0)
  {
    stanja[1] *= -1;
    if (stanja[1] > 0) transmit_data(02,41); //Pali H4 ID2
    else transmit_data(02,40); //Gasi H4 ID2
    while(digitalRead(36) == 0);
  }
  
  if(digitalRead(35) == 0)
  {
    stanja[2] *= -1;
    if (stanja[2] > 0) transmit_data(02,51); //Pali H5 ID2
    else transmit_data(02,50); // Gasi H5 ID2
    while(digitalRead(35) == 0);
  }
  
  if(digitalRead(34) == 0)
  {
    stanja[3] *= -1;
    if (stanja[3] > 0) transmit_data(02,61); //Pali H6 ID2
    else transmit_data(02,60); //Gasu H6 ID2
    while(digitalRead(34) == 0);
  }
  
  if(digitalRead(33) == 0)
  {
    stanja[4] *= -1;
    if (stanja[4] > 0) transmit_data(03,31); // Pali H3 ID3
    else transmit_data(03,30); //Gasi H3 ID3
    while(digitalRead(33) == 0);
  }
  
  if(digitalRead(32) == 0)
  {
    stanja[5] *= -1;
    if (stanja[5] > 0) transmit_data(03,41); // Pali H4 ID3
    else transmit_data(03,40); // Gasi H4 ID3
    while(digitalRead(32) == 0);
  }
  
  if(digitalRead(31) == 0)
  {
    stanja[6] *= -1;
    if (stanja[6] > 0) transmit_data(03,51); //Pali H5 ID3
    else transmit_data(03,50); //Gasi H5 ID3
    while(digitalRead(31) == 0);
  }
  
  if(digitalRead(30) == 0)
  {
    stanja[7] *= -1;
    if (stanja[7] > 0) transmit_data(03,61); //Pali H6 ID3
    else transmit_data(03,60); //Gasi H6 ID3
    while(digitalRead(30) == 0);
  }
}
