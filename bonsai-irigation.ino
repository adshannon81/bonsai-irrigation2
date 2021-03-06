/*
 * Bonsai Irrigation v2
 * 
 * Simplifies watering. Watering lets moisture drop to drier level, then water's to higher level.
 * Up/Down watering may be better for the plant than constant level moisture.
 * 
 * Button always waters when pressed.
 * 
 * Watering changed to small burst to allow better soaking.
 */

//for LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PUMP        3 
#define MOIST       A6        //analog pin for soil moisture
#define MOISTPOWER  4         //only power moisture reader when in use
#define BUTTON_PIN  2         //digital pin for button

int   moisture  = 0;
int   moistureRAW  = 0;
int   lowMoistureSetting = 500;
int   highMoistureSetting = 580;

int   waterLevelLow = 10; //only water when moisture is below this %
int   waterLevelHigh = 70; 

unsigned long currentMillis = 0;

byte buttonState = LOW; 
const int buttonInterval = 300;
unsigned long previousButtonMillis = 0; 

byte pumpState = LOW; 
bool pumping = false;
const unsigned long pumpInterval = 3600000 ;//1 hour -  10800000; //3 hours
const int pumpDuration = 500; // 0.5 seconds
const int soakingDuration = 4000; // 4 seconds
unsigned long previousPumpMillis = 0;

LiquidCrystal_I2C lcd(0x27,16,2);


void setup() {
  Serial.begin(9600);
  Serial.println("Starting ...");  // so we know what sketch is running

  lcd.init();
  lcd.backlight();//Power on the back light
 
  pinMode(PUMP, OUTPUT);
  pinMode(MOIST, INPUT);
  pinMode(MOISTPOWER, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  readMoisture();
  
}

void loop() {
  currentMillis = millis(); 

  readPumpButton();   
  updatePump_State();
  //readMoisture();
  writeLCD();

  reset();
  delay(1000);
}

void updatePump_State()
{  

  if(currentMillis - previousPumpMillis >= pumpInterval)
  {
    readMoisture();
    if(moisture <=waterLevelLow)
    {
      pumping = true;
      waterPlant();
    }
    else if(pumping)
    {
      if(moisture <= waterLevelHigh)
      {
        waterPlant();
       }
       else
       {
        pumping = false;
       }
        
    }
    else{
      writeLCD("Moist enough!");
    }
    previousPumpMillis += pumpInterval;
  }
  
}

void waterPlant()
{
  
  int pumpCount = 5;
  
  writeLCD("Pumping...");
  while(moisture <= waterLevelHigh)
  {
    digitalWrite(PUMP, HIGH);
    delay(pumpDuration * ( round(pumpCount / 5)) );  
    digitalWrite(PUMP, LOW);
  
    writeLCD("Soaking...");
    delay(soakingDuration);
    
    readMoisture();
    writeLCD("Moisture: " + String(moisture) + "%");// + String(moistureRAW));
    delay(1500);
    pumpCount++;
  }
  currentMillis = millis();
}


void readPumpButton() 
{ 
  if (millis() - previousButtonMillis >= buttonInterval) 
  {
    if (digitalRead(BUTTON_PIN) == HIGH) 
    {
      waterPlant();     
      previousButtonMillis += buttonInterval;
    }
  }
}

//reads level 5 times and gets the average.
void readMoisture()
{
  digitalWrite(MOISTPOWER, HIGH);
  moisture = 0;
  for (int i = 0; i < 5; i++) {
    moisture = moisture + analogRead(MOIST);
    delay(10);
  }
  moisture = moisture / 5;
  moistureRAW = moisture;
  
  //dry soil reading = 550, wet soil = 10; map to 0-100
  moisture = map(moisture,lowMoistureSetting,highMoistureSetting,0,100);
  
  digitalWrite(MOISTPOWER, LOW);
}

void writeLCD()
{
  lcd.clear();//Clean the screen

  lcd.setCursor(0, 0);
  lcd.print("Moisture: "); 
  lcd.print(moisture);
  lcd.print("%");
  //lcd.print(moistureRAW);
  lcd.setCursor(0, 1);

  unsigned long rem = ( ((previousPumpMillis + pumpInterval) - currentMillis)/1000) ;
  lcd.print("Check in ");
  if(rem >60)
  {
    rem = rem /60;
    lcd.print(rem);
    lcd.print(" min"); 
  }
  else 
  {
    lcd.print(rem);
    lcd.print(" sec");       
  }   
   
}

void writeLCD(String text)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(text);
  delay(3000);
  
  currentMillis = millis(); 
}


void reset()
{
  if(currentMillis > 4000000000) // reset ~46 days, before reaching Long limit.
  {    
    previousButtonMillis = 0; 
    previousPumpMillis = 0;
    
    currentMillis = 0; //86400000;  //1 day
  }  
}
