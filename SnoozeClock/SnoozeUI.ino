#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// State 0
void clockDisplay(){
  tft.fillScreen(Display_Color_Yellow);
  tft.setTextColor(Display_Color_Black);
  tft.setCursor(1,1);
  tft.print("Clock Screen: State 0"); 
  tft.setCursor(1,13);
  tft.print(&timeinfo, "%A, %B %d"); 
  tft.setCursor(1, 27); 
  tft.setTextSize(3);
  tft.print(&timeinfo, " %H:%M"); 
  tft.setTextSize(1);
}

// State 1
void messagesOverview() {

  // TODO: display message icon if new message, alarm if alarm is set :)
  
  tft.fillScreen(Display_Color_Green);
  tft.setTextColor(Display_Color_Black);
  tft.setCursor(1,1);
  tft.print("Message View: State 1"); 

  // print out all messages
  for(int i = 0; i < MAX_MESSAGES; i++){
    tft.setCursor(1, 10*(i+1)); 
    tft.print(messages[i]); 
  }
}

// State 2
void openMessageScreen() {
  tft.fillScreen(Display_Color_Red); 
  tft.setTextColor(Display_Color_White); 
  tft.setCursor(1,1);
  tft.print("Open Msg: State 2"); 
}

// ALARM SCREENS: States 3,4,5,6
// =============================

// State 3
void setAlarmScreen(){
  tft.fillScreen(Display_Color_White); 
  tft.setTextColor(Display_Color_Black); 
  tft.setCursor(1,1); 
  tft.print("Set Alarm: State 3"); 

  // Initial display options
  tft.setCursor(1, 13); 
  tft.setTextColor(Display_Color_Red); 
  tft.print("on "); 
  tft.setTextColor(Display_Color_Black);
  tft.print("6 : 30 weekdays"); 
  tft.setCursor(1, 23); 
  tft.print("off       all");
}

// State 4
void setHoursScreen(){
  
}

// State 5
void setMinutesScreen(){
  
}

// State 6
void setScheduleScreen(){
  
}
