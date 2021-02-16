#include "Fonts/LexendMega_Regular9pt7b.h"
#include "Fonts/LexendMega_Regular12pt7b.h"
#include "Fonts/LexendMega_Regular18pt7b.h"
#include "Fonts/LexendMega_Regular20pt7b.h"
#include "Fonts/LexendMega_Regular24pt7b.h"
#include "Fonts/LexendMega_Regular30pt7b.h"
#include "Fonts/LexendMega_Regular36pt7b.h"

// State 0
void clockDisplay() {
  // if date or hour change, full refresh
  // unread message: display message notification
  // Partial update for minutes change?

  Serial.println("clk disp");
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_RED);
  display.setFont(&LexendMega_Regular9pt7b);

  // Display day and date
  display.setCursor(0, -7);
  display.println();
  display.print(&timeinfo, "%A, %b "); // full weekday name, abreviated month name, day
  display.println(timeinfo.tm_mday % 32);
  Serial.println(display.getCursorX()); 
  Serial.println(display.getCursorY());
  
  // Display weather
  display.print("30*"); 
  // TODO: include correct weather icon, get weather data from API call

  // Display the current time
  display.setFont(&LexendMega_Regular36pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(23, 100);
  display.print(&timeinfo, "%I:%M"); // 12 hr time, minutes
  display.setFont(&LexendMega_Regular9pt7b);
  Serial.println(display.getCursorX()); 
  display.println(&timeinfo, "%p");
  display.update();
}

void updateTime(){
  display.setFont(&LexendMega_Regular36pt7b);
  display.setTextColor(GxEPD_BLACK);

  uint16_t box_x = 1;
  uint16_t box_y = 40;
  uint16_t box_w = display.width()-1;
  uint16_t box_h = 50;
  uint16_t cursor_y = box_y + box_h - 6;
  display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
//  display.setCursor(box_x, cursor_y);
//  display.print(&timeinfo, "%I:%M");
  display.updateWindow(box_x, box_y, box_w, box_h, true);
  delay(2000);
  Serial.println(&timeinfo, "%I:%M");

}

// State 1
void messagesOverview() {

  // TODO: display message icon if new message, alarm if alarm is set :)

}

// State 2
void openMessageScreen() {

  // TODO: format message, allow UI for response mechanism
}

// ALARM SCREENS: States 3,4,5,6
// =============================

// Helper Function: set alarm screen vars
// Called to init screen, partial refresh after this
void alarmScreenInit() {
  // Display current hour, min, AM?PM, schedule
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_RED);
  display.setFont(&LexendMega_Regular9pt7b);
  display.setCursor(0, -7);
  display.println();  
  display.println("Alarm Settings:"); 
  display.setTextColor(GxEPD_BLACK); 

  // on/off
  display.setCursor(0, 60);  
  // make current setting red
  if(alarmSet == true){
    display.setTextColor(GxEPD_RED);
    display.println("on");
    display.setTextColor(GxEPD_BLACK); 
    display.print("off  ");
  }
  else{
    display.setTextColor(GxEPD_BLACK);
    display.println("on");
    display.setTextColor(GxEPD_RED); 
    display.print("off  ");
  }

  // Saved hours
  display.setFont(&LexendMega_Regular24pt7b);
  display.print(alarmHr%12); 

  // Saved mins
  display.print(":"); 
  display.print(alarmMin); 

  // AM or PM
  uint16_t cursorX = display.getCursorX(); 
  uint16_t cursorY = display.getCursorY(); 
  display.setFont(&LexendMega_Regular9pt7b); 
  display.setCursor(cursorX+5, 60); 
  display.println("  AM"); 
  cursorY = display.getCursorY(); 
  display.setCursor(cursorX+5, cursorY); 
  display.print("  PM"); 
  
  if(alarmHr/12 == 1){
    // we are in the PM hours
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(cursorX+5, 60); 
    display.println("  AM"); 
    cursorY = display.getCursorY(); 
    display.setTextColor(GxEPD_RED);
    display.setCursor(cursorX+5, cursorY); 
    display.print("  PM"); 
  }
  else{
    display.setTextColor(GxEPD_RED);
    display.setCursor(cursorX+5, 60); 
    display.println("  AM"); 
    cursorY = display.getCursorY(); 
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(cursorX+5, cursorY); 
    display.print("  PM"); 
  }
  
  // Schedule
  if(alarmDays[0] == 1 && alarmDays[1] == 1){
    // Both sunday and monday alarms set, we know it must be "7day"
    display.setTextColor(GxEPD_RED);
    cursorX = display.getCursorX(); 
    display.setCursor(cursorX+10, 50); 
    display.println("7day"); 
    cursorY = display.getCursorY(); 
    display.setCursor(cursorX+10, cursorY);
    display.setTextColor(GxEPD_BLACK); // Return to black
    display.println("wkdy");
    cursorY = display.getCursorY(); 
    display.setCursor(cursorX+10, cursorY);
    display.print("wknd");
  }
  else if(alarmDays[0] == 0 && alarmDays[1] == 1){
    // Sunday off, Monday on, must then be "wkdy"
    display.setTextColor(GxEPD_BLACK);
    cursorX = display.getCursorX(); 
    display.setCursor(cursorX+10, 50); 
    display.println("7day"); 
    cursorY = display.getCursorY(); 
    display.setCursor(cursorX+10, cursorY);
    display.setTextColor(GxEPD_RED);
    display.println("wkdy");
    cursorY = display.getCursorY(); 
    display.setCursor(cursorX+10, cursorY);
    display.setTextColor(GxEPD_BLACK);
    display.print("wknd");
  }
  else {
    // Must default to "wknd"
    display.setTextColor(GxEPD_BLACK);
    cursorX = display.getCursorX(); 
    display.setCursor(cursorX+10, 50); 
    display.println("7day"); 
    cursorY = display.getCursorY(); 
    display.setCursor(cursorX+10, cursorY);
    display.println("wkdy");
    cursorY = display.getCursorY(); 
    display.setCursor(cursorX+10, cursorY);
    display.setTextColor(GxEPD_RED);
    display.print("wknd");
  }
  
}

// State 3
void setAlarmScreen() {
  alarmScreenInit();
  display.fillRect(0, 80, 20, 3, GxEPD_BLACK); // Underline to indicate setting alarm on/off
  display.update();
}

// State 4
void setHoursScreen() {
  display.updateWindow(box_x, box_y, box_w, box_h, true); // update part of the screen
  alarmScreenInit(); // May not be needed? How does partial refresh work in this context?

}

// State 5
void setMinutesScreen() {
  alarmScreenInit();

}

// State 6
void setScheduleScreen() {
  alarmScreenInit();

}
