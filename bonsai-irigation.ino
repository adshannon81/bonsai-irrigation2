/*
Updated version
Simplifies watering. Watering lets moisture drop to drier level, then water's to higher level.
Up/Down watering may be better for the plant than constant level moisture.

Button always waters when pressed.

Watering changed to small burst to allow better soaking.

/*

//for LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PUMP        3 
#define MOIST       A6        //analog pin for soil moisture
#define MOISTPOWER  4         //only power moisture reader when in use
#define BUTTON_PIN  2         //digital pin for button

int   moisture  = 0;
int   lowMoistureSetting = 225;
int   highMoistureSetting = 700;
const int moistureInterval = 30000;
unsigned long previousMoistureMillis = 0; 

int   waterLevelLow = 45; //only water when moisture is below this %
int   waterLevelHigh = 70; 

unsigned long currentMillis = 0;

byte buttonState = LOW; 
int pumpOn = 0;
const int buttonInterval = 300;
unsigned long previousButtonMillis = 0; 

byte pumpState = LOW; 
const unsigned long pumpInterval = 60000; //1 minute - 3600000 ;//1 hour -  10800000; //3 hours
const int pumpDuration = 1000; // 1 seconds
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

  readButton(); 
  readMoistureInterval();
  
  updatePump_State();
  //switchPower();
  
  writeLCD();
  //writeLCDDebug_pump();
  //writeLCDDebug_button();

  reset();
  delay(1000);
}

void updatePump_State()
{  
  if(moisture <= wateringLevelCritical)
  {
      switchPower();
      previousPumpMillis = currentMillis;
      
    /*
    //if(currentMillis - previousPumpMillis >= pumpInterval)
    //{
      pumpState = HIGH;
      pumpOn=1;
      switchPower();
      delay(1000);
      pumpState = LOW;
      switchPower();
      delay(4000); //1 sec burst every 5 seconds to give water time to soak in.
      previousPumpMillis = currentMillis - pumpInterval;
      previousPumpMillis += pumpInterval;
      //previousPumpMillis += 1;
    //}
    */
  }
  else if(pumpState == LOW)
  {
    //writeLCD1000("PUMP STATE LOW");
  
    if(currentMillis - previousPumpMillis >= pumpInterval)
    {
      readMoisture();
      if(moisture <=wateringLevel)
      {
        //pumpState = HIGH;
        //pumpOn=1;
        switchPower();
      }
      else{
        writeLCD("Moist enough!");
      }
      previousPumpMillis += pumpInterval;
    }
  }
  else
  {
    //writeLCD1000("PUMP STATE HIGH");
    
    //writeLCD1000(currentMillis - previousPumpMillis);
    if( currentMillis - previousPumpMillis >= pumpDuration)
    {
      pumpState = LOW;  
      //previousPumpMillis += pumpInterval;    
    }
  }
}

/*
void switchPower()
{
  digitalWrite(PUMP, pumpState); 
}*/
void switchPower()
{
  pumpOn=1;
  writeLCD("Pumping...");
  digitalWrite(PUMP, HIGH);
  delay(pumpDuration);  
  digitalWrite(PUMP, LOW);
  
  writeLCD("Soaking...");
  delay(soakingDuration);
  
  currentMillis = millis(); 
}

void waterPlant()
{
  pumpOn = 1;
  wrtieLCD("Pumping...");
  while(moisture <= waterLevelHigh)
  {
    digitalWrite(PUMP, HIGH);
    delay(pumpDuration);  
    digitalWrite(PUMP, LOW);
  
    writeLCD("Soaking...");
    delay(soakingDuration);
    
    readMoisture();
    writeLCD("Moisture: " + moisture + "%");
    delay(1500);
  }
  currentMillis = millis();
}


void readButton() 
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

  lcd.setCursor(0, 1);
  
  if(pumpState == HIGH)
  {
    lcd.print("pumping: ");
    lcd.print( (pumpDuration - (currentMillis - previousPumpMillis))/1000);
    lcd.print(" sec");
  }
  else
  {
    unsigned long rem = ( ((previousPumpMillis + pumpInterval) - currentMillis)/1000) ;
    lcd.print("Pump: ");
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
    
    //lcd.print(rem);
//    lcd.print( ((previousPumpMillis + pumpInterval) - currentMillis)/1000);
    //lcd.print(" sec");    
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

void writeLCDDebug_pump()
{
  lcd.clear();//Clean the screen
  lcd.setCursor(0, 0);
  if(pumpState == HIGH)
  {
    lcd.print("pH ");
  }
  else{
    lcd.print("pL ");
  }
  lcd.print(currentMillis);
  lcd.setCursor(0,1);
  lcd.print(previousPumpMillis);
  lcd.print(":");
  lcd.print(pumpInterval);
  lcd.print(":");
  lcd.print(pumpDuration);
}
void writeLCDDebug_button()
{
  buttonState = digitalRead(BUTTON_PIN);
  
  lcd.clear();//Clean the screen
  lcd.setCursor(0, 0);
  if(buttonState == HIGH)
  {
    lcd.print("bH ");
  }
  else{
    lcd.print("bL ");
  }
  lcd.print(currentMillis);
  lcd.setCursor(0,1);
  lcd.print(previousButtonMillis);
  lcd.print(":");
  lcd.print(buttonInterval);
}

void reset()
{
  if(currentMillis > 4000000000) // reset ~46 days, before reaching Long limit.
  {    
    previousButtonMillis = 0; 
    previousPumpMillis = 0;
    previousMoistureMillis = 0;
    
    currentMillis = 0; //86400000;  //1 day
  }  
}
