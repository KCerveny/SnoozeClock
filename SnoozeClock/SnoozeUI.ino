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

// on/off display update
void onOff(){
  display.setFont(&LexendMega_Regular9pt7b);
  display.setCursor(0, 60);
  // make current setting red
  if(alarmSet == true){
    display.setTextColor(GxEPD_RED);
    display.println("on");
    display.setTextColor(GxEPD_BLACK); 
    display.print("off");
  }
  else{
    display.setTextColor(GxEPD_BLACK);
    display.println("on");
    display.setTextColor(GxEPD_RED); 
    display.print("off");
  }
  display.setTextColor(GxEPD_BLACK); // Reset for the coming text
}

// Hours Display helper
void hours(){
  display.setFont(&LexendMega_Regular24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.fillRect(42,40,58,60, GxEPD_WHITE); 
  display.setCursor(42, 82);
  if(alarmHr == 12 || alarmHr == 0){
    display.print(12); 
  }
  else{
    display.print(alarmHr%12); 
  }
}

// Minutes Display Helper
void mins(){
  display.setFont(&LexendMega_Regular24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.fillRect(103,40,88,60, GxEPD_WHITE); 
  display.setCursor(103, 82);
  display.print(":"); 
  display.print(alarmMin); 
}

// AM/PM Display Helper
void AMPM(){
  display.setFont(&LexendMega_Regular9pt7b); 
    
  if(alarmHr/12 == 1){
    // we are in the PM hours
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(193, 60); 
    display.println("AM"); 
    display.setTextColor(GxEPD_RED);
    display.setCursor(193, 82); 
    display.print("PM"); 
  }
  else{
    display.setTextColor(GxEPD_RED);
    display.setCursor(193, 60); 
    display.println("AM");  
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(193, 82); 
    display.print("PM"); 
  }
  Serial.print("Cursor after AMPM: ");
  Serial.print(display.getCursorX());
  Serial.print(", "); 
  Serial.println(display.getCursorY());
}

// Helper Display Schedule
void schedule(){
  uint16_t cursorX = 220;
  uint16_t cursorY;
  display.setFont(&LexendMega_Regular9pt7b); 
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
  // Helper functions to display all variables
  onOff();
  hours();
  mins();
  AMPM();
  schedule();
}

// State 3
void setAlarmScreen() {
  alarmScreenInit();
  display.fillRect(0, 100, 40, 3, GxEPD_BLACK); // Underline to indicate setting alarm on/off
  display.update();

  // DEBUGGING ONLY
  delay(3000);
  alarmSet = alarmSet ^ 1; // Toggle to opposite value
  setHoursScreen();
}

// State 4
void setHoursScreen() {
  onOff(); // Update on/off setting
  mins(); // Update mins if moving backwards
  display.fillRect(0, 100, 296, 3, GxEPD_WHITE); // remove all other underlines
  display.fillRect(42,100, 58, 3, GxEPD_BLACK); // add hours underline
  display.updateWindow(0,35,191,65,true); // Update from beginning of onOff to end of mins

  // DEBUGGING ONLY
  delay(3000);
  alarmHr = 12; // Toggle to opposite value
  setMinutesScreen();
}

// State 5
void setMinutesScreen() {
  hours(); // Update hours setting
  AMPM(); // Update if moving backwards
  display.fillRect(0, 100, 296, 3, GxEPD_WHITE); // remove all other underlines
  display.fillRect(103,100, 88, 3, GxEPD_BLACK); // add mins underline
  display.updateWindow(42,35,185,65,true); // Update from beginning of hours to end of AMPM

  // DEBUGGING ONLY
  delay(3000);
  alarmHr = 12; // Toggle to opposite value
  setAMPMScreen();
}

// TODO: we are missing the AMPM state for setting this alarm
void setAMPMScreen(){
  mins();
  schedule();
  display.fillRect(0, 100, 296, 3, GxEPD_WHITE); // remove all other underlines
  display.fillRect(193,100, 34, 3, GxEPD_BLACK); // add AMPM underline
  display.updateWindow(103,35,193,65,true); // Update from beginning of hours to end of schedule

  // DEBUGGING
  delay(1000);
  setScheduleScreen();
}

// State 6
void setScheduleScreen() {
  AMPM(); 
  display.fillRect(0, 100, 296, 3, GxEPD_WHITE); // remove all other underlines
  display.fillRect(228,100, 68, 3, GxEPD_BLACK); // add underline

}
