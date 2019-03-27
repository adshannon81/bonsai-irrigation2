

//for LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PUMP        3 
#define MOIST       A6        //analog pin for soil moisture
#define MOISTPOWER  4
#define BUTTON_PIN  2         //digital pin for button

int   moisture  = 0;
int   lowMoistureSetting = 225;
int   highMoistureSetting = 700;
const int moistureInterval = 30000;
unsigned long previousMoistureMillis = 0; 

int   wateringLevel = 50; //only water when moisture is below this %
int   wateringLevelCritical = 35; //if sudden drop in moisture, don't weight for pump timer.

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


void readButton() {
  // this only reads the button state after the button interval has elapsed
  //  this avoids multiple flashes if the button bounces
  // every time the button is pressed it changes buttonLed_State causing the Led to go on or off
  // Notice that there is no need to synchronize this use of millis() with the flashing Leds
 
  if (millis() - previousButtonMillis >= buttonInterval) 
  {
    if (digitalRead(BUTTON_PIN) == HIGH) 
    {
      //pumpState = ! pumpState; 
      //pumpState = LOW;
      previousPumpMillis = currentMillis - pumpInterval;

     
      previousButtonMillis += buttonInterval;
    }
  }
}

void readMoisture()
{
  digitalWrite(MOISTPOWER, HIGH);
  delay(20);
  moisture = analogRead(MOIST);
  //dry soil reading = 550, wet soil = 10; map to 0-100
  moisture = map(moisture,lowMoistureSetting,highMoistureSetting,0,100);
  
  digitalWrite(MOISTPOWER, LOW);
}

void readMoistureInterval()
{
  if(currentMillis - previousMoistureMillis >= moistureInterval)
  {
    readMoisture();
    previousMoistureMillis += moistureInterval;
  }
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
