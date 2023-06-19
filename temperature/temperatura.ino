
#include <Adafruit_BME280.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 

#include <EEPROM.h>

#define TFT_CS        10
#define TFT_RST        9 
#define TFT_DC         8

Adafruit_BME280 bme;

Adafruit_ST7735 tft=Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

volatile float currentTemp;
float prevTemp, changeDif;

unsigned int setTemp;

bool state, dif;

const uint8_t addr = 0;
const uint8_t addr2 = 1;
const uint8_t addr3 = 2;

void increase()
{
  if(setTemp < 35)
  {
    setTemp += 1;
    updateSetTemp();
  } 
}

void decrease()
{
  if(setTemp > 15)
  {
    setTemp -= 1;
    updateSetTemp();
  }
}

void updateCurrentTemp()
{
  tft.fillRect(10, 21, 120, 40, ST7735_BLUE);
  tft.setCursor(10, 21);
  tft.setTextSize(5);
  tft.print(currentTemp, 1);
}

void updateSetTemp()
{
  tft.fillRect(10, 80, 55, 40, ST7735_BLUE);
  tft.setCursor(10, 80);
  tft.setTextSize(5);
  tft.print(setTemp);
}

void save()
{
  EEPROM.write(addr, setTemp);
}

void changeButtonColor()
{
  if(state)
  {
    tft.fillRect(118, 69, 34, 54, ST7735_GREEN);
    tft.setCursor(125, 80);
    tft.setTextSize(1);
    tft.print("ON");
  }
  else
  {
    tft.fillRect(118, 69, 34, 54, ST7735_RED);
    tft.setCursor(125, 80);
    tft.setTextSize(1);
    tft.print("OFF");
  }
}

float truncF(float num)
{
  int temp;
  temp = (int)(num*10);
  return (float)temp/10;
}

void updateDif()
{
  if(changeDif == 0.5)
  {
    changeDif = 0.0;
    tft.fillCircle(100, 117 ,4, ST7735_RED);
    dif = 1;
  }
  else
  {
    changeDif = 0.5;
    tft.fillCircle(100, 117 ,4, ST7735_GREEN);
    dif = 0;
  }
  state = !state;
  changeButtonColor();
}

void compareTemp()
{
  if(currentTemp > setTemp)
  {
    if(state)
    {
      state = 0;
      changeButtonColor();
      digitalWrite(5, LOW);
      Serial.println("currentTemp > setTemp");
    }
    return;
  }
  if(currentTemp + changeDif < setTemp)
    {
      if(!state)
      {
        state = 1;
        changeButtonColor();
        digitalWrite(5, HIGH);
        Serial.println("currentTemp + changeDif < setTemp");
      }
      return;
   } 
}


void setup() {
  Serial.begin(9600);  
  bme.begin(0x76);

  int screenWidth = 160;
  int screenHeight = 128;  
  
  //Inicializacija zaslona na belo barvo
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3); //obrne zaslon na pravo stran
  tft.fillScreen(ST7735_GREEN);
  tft.setTextSize(1);
  tft.fillRect(1, 1, screenWidth-2, (screenHeight-2)/2-1, ST7735_BLUE);
  tft.fillRect(1, (screenHeight-2)/2+1, screenWidth-2, (screenHeight-2)/2, ST7735_BLUE);
  tft.setCursor(10, 6);
  tft.print("TEMPERATURA V PROSTORU");
  tft.setCursor(10, 65);
  tft.print("NASTAVLJENA TEMP");


  // Znak °C
  tft.setTextSize(3);
  tft.setCursor(135, 21);
  tft.print("C");
  tft.setCursor(75, 80);
  tft.print("C");
  tft.setTextSize(1);
  tft.setCursor(130, 17);
  tft.print("o");
  tft.setCursor(70, 76);
  tft.print("o");
  
  tft.drawRect(115, 66, 40, 60, ST7735_WHITE); 
  tft.drawRect(116, 67, 38, 58, ST7735_WHITE);
  tft.drawRect(117, 68, 36, 56, ST7735_BLACK);

  tft.drawCircle(100, 117 ,6, ST7735_WHITE);
  tft.drawCircle(100, 117 ,5, ST7735_WHITE);
 
  currentTemp = truncF(bme.readTemperature());

  state = 0;
  changeDif = 0.5;
  
  if(EEPROM.read(addr) != 255)
    setTemp = EEPROM.read(addr);
  else
    setTemp = 23;
    
  updateSetTemp();
  updateCurrentTemp();
  changeButtonColor();
  updateDif();
  
  attachInterrupt(digitalPinToInterrupt(2), decrease, RISING);
  attachInterrupt(digitalPinToInterrupt(3), increase, RISING);

  pinMode(5, OUTPUT);
}


void loop() {
  prevTemp = currentTemp;
  currentTemp = truncF(bme.readTemperature());
  if(analogRead(A2) == 0)
    updateDif();
  if(currentTemp != prevTemp)
    updateCurrentTemp();
  compareTemp();
  save();
  delay(500);
}
