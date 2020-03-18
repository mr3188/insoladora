//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define BT_UP_PIN     9
#define BT_DOWN_PIN   10
#define BT_LEFT_PIN   11
#define BT_RIGHT_PIN  12

#define BT_PUSH_DELAY 200

enum Mode{
  config,
  setMinutes,
  setSeconds,
  started,
  finished
};


const char* modeNames[]= {"Configuration.  ",
                          "Set minutes.    ",
                          "Set seconds.    ",
                          "Running...      ",
                          "Finished.       "};

Mode  mode;

int time=0;
int seconds=0;
int minutes=0;

boolean running=false;
boolean setTime=true;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

byte upArrow[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte downArrow[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

byte rightArrow[] = {
  B00000,
  B00100,
  B00110,
  B11111,
  B00110,
  B00100,
  B00000,
  B00000
};

byte leftArrow[] = {
  B00000,
  B00100,
  B01100,
  B11111,
  B01100,
  B00100,
  B00000,
  B00000
};

enum { UP_ARROW_CHAR, DOWN_ARROW_CHAR, LEFT_ARROW_CHAR, RIGHT_ARROW_CHAR};

// display all keycodes
void displayKeyCodes(void) {
  uint8_t i = 0;
  while (1) {
    lcd.clear();
    lcd.print("Codes 0x"); lcd.print(i, HEX);
    lcd.print("-0x"); lcd.print(i+16, HEX);
    lcd.setCursor(0, 1);
    for (int j=0; j<16; j++) {
      lcd.printByte(i+j);
    }
    i+=16;
    
    delay(4000);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting...");
  pinMode(BT_UP_PIN, INPUT_PULLUP);
  pinMode(BT_DOWN_PIN, INPUT_PULLUP);
  pinMode(BT_LEFT_PIN, INPUT_PULLUP);
  pinMode(BT_RIGHT_PIN, INPUT_PULLUP);

  time=0;
  mode=Mode::config;
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.home();
  lcd.blink();

  lcd.createChar(UP_ARROW_CHAR, upArrow);
  lcd.createChar(DOWN_ARROW_CHAR, downArrow);
  lcd.createChar(LEFT_ARROW_CHAR, leftArrow);
  lcd.createChar(RIGHT_ARROW_CHAR, rightArrow);
}

void stop()
{
  running=false;
}

void go()
{
  running=true;
}


void displayTime()
{  
    lcd.setCursor(0, 1);

    int minutes=time/60;
    
    if(minutes<10)
      lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");

    int seconds=time-(minutes*60);

    if(seconds<10)
      lcd.print("0");

    lcd.print(seconds);
    lcd.print(" ");
}


void loop()
{
  
  lcd.home();
  lcd.print(modeNames[mode]);


  switch (mode)
  {
  case Mode::config:    
    mode=Mode::setSeconds;
    break;

  case Mode::setSeconds:
    lcd.setCursor(0, 5);
    

    if(!digitalRead(BT_UP_PIN))
      if(seconds<60)
        seconds++;

    if(!digitalRead(BT_DOWN_PIN))
      if(seconds>0)
        seconds--;

    if(!digitalRead(BT_LEFT_PIN))
    {
      Serial.println("set mode setMinutes");
      mode=Mode::setMinutes;
    }
      
    
    delay(BT_PUSH_DELAY);
    time=(minutes*60)+seconds;
    displayTime();
    break;

  case Mode::setMinutes:

    lcd.setCursor(0, 2);

    if(!digitalRead(BT_UP_PIN)&& minutes<99)      
        minutes++;

    if(!digitalRead(BT_DOWN_PIN)&& minutes>0)
        minutes--;
    
    if(!digitalRead(BT_RIGHT_PIN))
    {
      Serial.println("set mode setSeconds");
      mode=Mode::setSeconds;
    }
      

    delay(BT_PUSH_DELAY);
    time=(minutes*60)+seconds;
    displayTime();

    if(!digitalRead(BT_LEFT_PIN))
    {    
      Serial.println("Start");
      Serial.flush();
      lcd.setCursor(0, 0);
      lcd.write(UP_ARROW_CHAR);
      lcd.print("Start ");
      lcd.write(DOWN_ARROW_CHAR);
      lcd.write(LEFT_ARROW_CHAR);
      lcd.write(RIGHT_ARROW_CHAR);
      lcd.println("Cancel");

      delay(BT_PUSH_DELAY);    
      while(digitalRead(BT_UP_PIN) && digitalRead(BT_DOWN_PIN) && digitalRead(BT_LEFT_PIN) && digitalRead(BT_RIGHT_PIN));
      
      if(!digitalRead(BT_UP_PIN))
      {
          Serial.println("set mode setStarted");
          mode=Mode::started;
      }
    }
     
    delay(BT_PUSH_DELAY);
    break;
  
  case Mode::started:    
      time--;
      displayTime();
      delay(1000);

      if(time==0)
      {
          stop();
          mode=Mode::finished;
      }
        

    break;

  case Mode::finished:
    lcd.noBacklight();
    delay(BT_PUSH_DELAY);
    lcd.backlight();

    while(digitalRead(BT_UP_PIN) && digitalRead(BT_DOWN_PIN) && digitalRead(BT_LEFT_PIN) && digitalRead(BT_RIGHT_PIN));
    mode=Mode::config;


  default:    
    break;
  }
}