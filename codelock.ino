/*
 *  Application note: Simple Keylock / Keypad for ArduiTouch and ESP32  
 *  Version 1.0
 *  Copyright (C) 2019  Hartmut Wendt  www.zihatec.de
 *  
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/   

 

/*______Import Libraries_______*/
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "usergraphics.h"
/*______End of Libraries_______*/


/*__Pin definitions for the ESP8266__*/
#define TFT_CS   5
#define TFT_DC   4
#define TFT_LED  15  
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  22
#define TFT_MISO 19
#define TFT_LED  15  

#define HAVE_TOUCHPAD
#define TOUCH_CS 14
#define TOUCH_IRQ 2

/*_______End of definitions______*/

 

/*____Calibrate Touchscreen_____*/
#define MINPRESSURE 10      // minimum required force for touch event
#define TS_MINX 370
#define TS_MINY 470
#define TS_MAXX 3700
#define TS_MAXY 3600
/*______End of Calibration______*/


/*___Keylock spezific definitions___*/
#define codenum 42    // 42 is the answer for everything, but you can change this to any number between 0 and 999999

/*___End of Keylock spezific definitions___*/


//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//XPT2046_Touchscreen touch(TOUCH_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);



String symbol[4][4] = {
  { "7", "8", "9" },
  { "4", "5", "6" },
  { "1", "2", "3" },
  { "C", "0", "OK" }
};
int X,Y;
long Num1,Num2,Number;
char action;
boolean result = false;
bool Touch_pressed = false;
TS_Point p;


void setup() {
  Serial.begin(115200); //Use serial monitor for debugging

  pinMode(TFT_LED, OUTPUT); // define as output for backlight control

  Serial.println("Init TFT and Touch...");
  tft.begin();
  touch.begin();
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  
  IntroScreen();
  digitalWrite(TFT_LED, LOW);    // LOW to turn backlight on; 

  delay(1500);

  digitalWrite(TFT_LED, HIGH);    // HIGH to turn backlight off - will hide the display during drawing
  draw_BoxNButtons(); 
  digitalWrite(TFT_LED, LOW);    // LOW to turn backlight on; 

  //sound configuration
  ledcSetup(0,1E5,12);
  ledcAttachPin(21,0);

}

void loop() {
  // check touch screen for new events
  if (Touch_Event()== true) { 
    X = p.y; Y = p.x;
    Touch_pressed = true;
    
  } else {
    Touch_pressed = false;
  }

  // if touch is pressed detect pressed buttons
  if (Touch_pressed == true) {
    
    DetectButtons();
  
    if (result==true) {
      if (Number == codenum) {
        draw_Result_Box(ILI9341_GREEN,"CODE OK",60);
        ledcWriteTone(0,1000);
        delay(800);
        ledcWriteTone(0,0);
      } else {
        draw_Result_Box(ILI9341_RED, "WRONG CODE",30);
        for (int i=0;i< 3;i++) {
          ledcWriteTone(0,4000);
          delay(100);
          ledcWriteTone(0,0);
          delay(50);      
        }
      }
      delay(1000);
      Number = 0; 
      result=false;
    }

    DisplayResult(); 
  }    
  delay(100);
  ledcWriteTone(0,0);

}

/********************************************************************//**
 * @brief     detects a touch event and converts touch data 
 * @param[in] None
 * @return    boolean (true = touch pressed, false = touch unpressed) 
 *********************************************************************/
bool Touch_Event() {
  p = touch.getPoint(); 
  delay(1);
  #ifdef touch_yellow_header
    p.x = map(p.x, TS_MINX, TS_MAXX, 320, 0); // yellow header
  #else
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320); // black header
  #endif
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
  if (p.z > MINPRESSURE) return true;  
  return false;  
}



/********************************************************************//**
 * @brief     detecting pressed buttons with the given touchscreen values
 * @param[in] None
 * @return    None
 *********************************************************************/
void DetectButtons()
{
  if (X>185) //Detecting Buttons on Column 1
  {
    if (Y>265) //If cancel Button is pressed
    {Serial.println ("Button Cancel"); Number=Num1=Num2=0; result=false;}
    
     if (Y>205 && Y<255) //If Button 1 is pressed
    {Serial.println ("Button 1"); 
    Button_ACK_Tone();
    if (Number==0)
    Number=1;
    else
    Number = (Number*10) + 1; //Pressed twice
    }
    
     if (Y>145 && Y<195) //If Button 4 is pressed
    {Serial.println ("Button 4"); 
    Button_ACK_Tone();
    if (Number==0)
    Number=4;
    else
    Number = (Number*10) + 4; //Pressed twice
    }
    
     if (Y>85 && Y<135) //If Button 7 is pressed
    {Serial.println ("Button 7");
    Button_ACK_Tone();
    if (Number==0)
    Number=7;
    else
    Number = (Number*10) + 7; //Pressed twice
    } 
  }

  if (X<175 && X>85) //Detecting Buttons on Column 2
  {
    if (Y>265)
    {Serial.println ("Button 0"); //Button 0 is Pressed
    Button_ACK_Tone();
    if (Number==0)
    Number=0;
    else
    Number = (Number*10) + 0; //Pressed twice
    }
    
    if (Y>205 && Y<255)
    {Serial.println ("Button 2"); 
    Button_ACK_Tone();
     if (Number==0)
    Number=2;
    else
    Number = (Number*10) + 2; //Pressed twice
    }
    
     if (Y>145 && Y<195)
    {Serial.println ("Button 5"); 
    Button_ACK_Tone();
     if (Number==0)
    Number=5;
    else
    Number = (Number*10) + 5; //Pressed twic
    }
    
     if (Y>85 && Y<135)
    {Serial.println ("Button 8"); 
    Button_ACK_Tone();
     if (Number==0)
    Number=8;
    else
    Number = (Number*10) + 8; //Pressed twic
    }   
  }

  if (X>0 && X<75) //Detecting Buttons on Column 3
  {
    if (Y>265)
    {Serial.println ("Button OK"); 
    result = true;
    }
    
     if (Y>205 && Y<255)
    {Serial.println ("Button 3"); 
    Button_ACK_Tone();
    if (Number==0)
    Number=3;
    else
    Number = (Number*10) + 3; //Pressed twice
    }
    
     if (Y>145 && Y<195)
    {Serial.println ("Button 6"); 
    Button_ACK_Tone();
    if (Number==0)
    Number=6;
    else
    Number = (Number*10) + 6; //Pressed twice
    }
    
     if (Y>85 && Y<135)
    {Serial.println ("Button 9");
    Button_ACK_Tone();
    if (Number==0)
    Number=9;
    else
    Number = (Number*10) + 9; //Pressed twice
    }   
  }

}


/********************************************************************//**
 * @brief     shows the entered numbers (stars)
 * @param[in] None
 * @return    None
 *********************************************************************/
void DisplayResult()
{
    String s1="";
    //String s2  = String(Number);
    tft.fillRect(0, 0, 240, 80, ILI9341_CYAN);  //clear result box
    tft.setCursor(10, 20);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK);
    if (Number == 0) {
      tft.println(" ");
    } else { 
       for (int i=0;i< String(Number).length();i++)
       s1 = s1 + "*";
       tft.println(s1); //update new value
    }   
}

/********************************************************************//**
 * @brief     shows the intro screen in setup procedure
 * @param[in] None
 * @return    None
 *********************************************************************/
void IntroScreen()
{
  //Draw the Result Box
  tft.fillRect(0, 0, 240, 320, ILI9341_WHITE);
  tft.drawRGBBitmap(20,80, Zihatec_Logo,200,60);
  tft.setTextSize(0);
  tft.setTextColor(ILI9341_BLACK);
  tft.setFont(&FreeSansBold9pt7b);  

  tft.setCursor(45, 190);
  tft.println("ArduiTouch ESP");
  
  tft.setCursor(43, 215);
  tft.println("Keylock example");

}


/********************************************************************//**
 * @brief     draws a result box after code confirmation with ok button
 * @param[in] color background color of result box
 * @param[in] test  string to display in result box area
 * @param[in] xPos  X position of text output
 * @return    None
 *********************************************************************/
void draw_Result_Box(int color, char text[10], int xPos)
{
   //Draw the Result Box
   tft.fillRect(0, 0, 240, 80, color);
   tft.setCursor(xPos, 26);
   tft.setTextSize(3);
   tft.setTextColor(ILI9341_WHITE);

   // draw text
   tft.println(text);
}

/********************************************************************//**
 * @brief     draws the keypad
 * @param[in] None
 * @return    None
 *********************************************************************/
void draw_BoxNButtons()
{
  
   //clear screen black
  tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);
  tft.setFont(0);  
  
  //Draw the Result Box
  tft.fillRect(0, 0, 240, 80, ILI9341_CYAN);

  //Draw C and OK field   
  tft.fillRect  (0,260,80,60,ILI9341_RED);
  tft.fillRect  (160,260,80,60,ILI9341_GREEN); 
  
  //Draw Horizontal Lines
  for (int h=80; h<=320; h+=60)
  tft.drawFastHLine(0, h, 240, ILI9341_WHITE);

  //Draw Vertical Lines
  for (int v=80; v<=240; v+=80)
  tft.drawFastVLine(v, 80, 240, ILI9341_WHITE);

  //Display keypad lables 
  for (int j=0;j<4;j++) {
    for (int i=0;i<3;i++) {
      tft.setCursor(32 + (80*i), 100 + (60*j)); 
      if ((j==3) && (i==2)) tft.setCursor(24 + (80*i), 100 + (60*j)); //OK button
      tft.setTextSize(3);
      tft.setTextColor(ILI9341_WHITE);
      tft.println(symbol[j][i]);
    }
  }
}


/********************************************************************//**
 * @brief     plays ack tone (beep) after button pressing
 * @param[in] None
 * @return    None
 *********************************************************************/
void Button_ACK_Tone(){
  ledcWriteTone(0,4000);
}
