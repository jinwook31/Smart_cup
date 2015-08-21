#include <SoftwareSerial.h>
#include "OneWire.h"
#include "Adafruit_NeoPixel.h"
#include <avr/power.h>

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(16, 5, NEO_GRB + NEO_KHZ800);

OneWire  ds(4);
SoftwareSerial BT(2, 3);

byte addr[8];
uint32_t ds_Timer = 0;
boolean ds_count = false;
float celsius;
int Data[5] = {
  0,};

void setup()
{
  BT.begin(9600);
  Serial.begin(9600);
  pixels.begin();
}
void loop()
{
  temp_in();
  Data[0] = temp_out();
  Data[1] = accx();
  Data[2] = accy();
  Data[3] = Gr();
  Data[4] = state();
  led(Data[4]);
  convert(Data[0]);
  TxRx();
}
void convert(int N)
{
  o = Data[N]/100;
  Data[N] -= o*100;
  BT.write(o+48);
  
  o = Data[N]/10;
  Data[N] -= o*10;
  BT.write(o+48);
  
  o = Data[N];
  BT.write(o+48);
}
void TxRx()
{
  char Take = 0;
  if(BT.available())
  {
    Take = BT.read();
    Serial.println(Take);
  }
  BT.flush();
  switch(Take)
  {
  case 't' :
    BT.write(Data[0]);
    break;
  case 'x' :
    BT.write(Data[1]);
    break;
  case 'y' :
    BT.write(Data[2]);
    break;
  case 'g' :
    BT.write(Data[3]);
    break;
  case 's' :
    BT.write(Data[4]);
    break;
  default :
    break;
  }
}
void led(int N)
{
  if(N == 0)
    pixels.setPixelColor(15, pixels.Color(255,0,0));
  else if(N == 1)
    pixels.setPixelColor(15, pixels.Color(0,255,0));
  else if(N == 2)
    pixels.setPixelColor(15, pixels.Color(0,0,255));
  pixels.show();
}

int Gr()
{
  return analogRead(3);
}
int accy()
{
  return analogRead(1);
}
int accx()
{
  return analogRead(0);
}
int state()
{
  int sat = 0;
  int regist = analogRead(2);
  regist = constrain(regist,50,940);
  return ((regist-50)/300);
}
void temp_in()
{
  if(ds_count == false)
  {
    ds.search(addr);
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);
    ds_Timer = millis();
    ds_count = true;
  }
}
float temp_out()
{
  if(millis() - ds_Timer > 100)
  {
    ds_count = false;
    byte data[12];
    // we might do a ds.depower() here, but the reset will take care of it.

    ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad

    for (int i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
    }

    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    celsius = (float)raw / 16.0;
    return celsius;
  }
  return celsius;
}


