#include <RTClock.h>
#include <SoftWire.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

RTClock rtclock(RTCSEL_LSE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

time_t currentTime;
int8_t X = 0, x = 0, Y = 0, y = 0, incomingByte = 0, playText = 0;
uint8_t counter, messageSize = 28;
char scrollingText[255] = "Zdes mogla byt washa reklama";

void setup() {
  Serial1.begin(9600);

  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, HIGH);

  lcd.init();
  lcd.backlight();

  Serial1.println("For time setup write '/', for text setup write ':'");
}

void loop() {
  if(playText==7){
    int8_t counterBeginText = 0, counterEndText = 0, counterLCD = 15;

    while(counterLCD!=-1){//Цикл перемещает первый символ текста, пока он не дойдёт до начала дисплея
      lcd.clear();
      lcd.setCursor(counterLCD,1);
      for(counterBeginText; counterBeginText < counterEndText; counterBeginText++){
        lcd.print(scrollingText[counterBeginText]);
      }
      counterLCD -= 1;
      counterBeginText = 0;
      counterEndText = (counterEndText == messageSize) ? counterEndText : counterEndText + 1;
      delay(300);
    }

    counterBeginText += 1;

    while(counterBeginText!=counterEndText){//Аналогично верхнему, но пока последний символ не дойдёт до начала
      lcd.clear();
      lcd.setCursor(0,1);
      counter = counterBeginText;
      for(counter; counter < counterEndText; counter++){
        lcd.print(scrollingText[counter]);
      }
      counterBeginText +=1;
      counterEndText = (counterEndText == messageSize) ? counterEndText : counterEndText + 1;
      delay(300);
    }
  }

  static time_t lastTime = 0;
  currentTime = rtclock.getTime();
  
  if (currentTime != lastTime) {
    lastTime = currentTime;
    lcd.clear();
    
    tm_t timeInfo;
    rtclock.breakTime(currentTime, timeInfo);

    char timeString[9];
    sprintf(timeString, "%02d:%02d", 
            timeInfo.hour + X + x, 
            timeInfo.minute + Y + y);

    lcd.setCursor(0,0);
    lcd.print(timeString);
  }

  if(Serial1.available() > 0){
    incomingByte = Serial1.read();
    if(incomingByte == 47){
      Serial1.println("Time setup, write 'n' for next step on LCD, write '+' for set time, write '*' for end setup");
      int8_t col = 0;
      lcd.blink();

      while(incomingByte != 42){
        lcd.setCursor(col,0);

        incomingByte = Serial1.read();

        switch (incomingByte) {
        case 42:
          Serial1.println("End time setup");
          lcd.noBlink();
          break;

        case 43:
          switch(col){
            case 0:
            X = (X == 20) ? 0 : X + 10;
            break;

            case 1:
            if (X == 20) {
              x = (x == 3) ? 0 : x + 1;
            } else {
              x = (x == 9) ? 0 : x + 1;
            }
            break;

            case 3:
            Y = (Y == 50) ? 0 : Y + 10;
            break;

            case 4:
            y = (y == 9) ? 0 : y + 1;
            break;

          }
          break;

        case 110:
          col = (col + 1) % 5;
          break;
        }

        digitalWrite(PC13, LOW);
        delay(100);
        digitalWrite(PC13, HIGH);
        delay(100);
      }
    }
    else if(incomingByte == 58){
      Serial1.println("Set text, for end write '.'");
      counter = 0;
      while(incomingByte != 46){
        if(Serial1.available() > 0 && counter != 255){//Текст максимум на 254 символа, чтобы не перейти за границу буфера
          incomingByte = Serial1.read();
          scrollingText[counter] = (incomingByte == 46) ? '\0' : incomingByte;
          counter++;
        }
        digitalWrite(PC13, LOW);
        delay(100);
        digitalWrite(PC13, HIGH);
        delay(100);
      }
      messageSize = counter - 1;
      Serial1.println("End text setup");
      playText = 6;
    }
  }

  delay(3000);
  playText = (playText + 1) % 8;//Раз в 21 секунду будет текст появляться 
}