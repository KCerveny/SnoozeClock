#include "Fonts/LexendMega_Regular9pt7b.h"
#include "Fonts/LexendMega_Regular12pt7b.h"
#include "Fonts/LexendMega_Regular18pt7b.h"
#include "Fonts/LexendMega_Regular20pt7b.h"
#include "Fonts/LexendMega_Regular24pt7b.h"
#include "Fonts/LexendMega_Regular30pt7b.h"
#include "Fonts/LexendMega_Regular36pt7b.h"
#include "weatherIcons.h"
#include "miscIcons.h"

// State 0
void clockDisplay() {
  // if date or hour change, full refresh
  // unread message: display message notification
  // Partial update for minutes change?

  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK); // was GxEPD_RED
  display.setFont(&LexendMega_Regular9pt7b);

  // Display day and date
  display.setCursor(0, -7);
  display.println();
  display.print(&timeinfo, "%A, %b "); // full weekday name, abreviated month name, day
  display.println(timeinfo.tm_mday % 32);
  
  // Display weatherIcons
  display.setTextColor(GxEPD_RED);
  display.print(temp+"*F "); 
  display.setTextColor(GxEPD_BLACK);
  display.print(precip+"%");
//  display.drawBitmap(gridicons_ink, display.getCursorX()+3, display.getCursorY()-3, 24, 24, GxEPD_BLACK);
  
  showWeatherIcon();

  // Display notification icons
  if(alarmSet == 1){
    display.drawBitmap(gridicons_bell, 272, 0, 24, 24, GxEPD_BLACK);
  }
  if(digitalRead(inbox) == HIGH){
    display.drawBitmap(gridicons_mail, 247, 0, 24, 24, GxEPD_RED);
  }
  
  
  // Display the current time
  display.setFont(&LexendMega_Regular36pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(12, 108); // Originally at (23, 100)
  display.print(&timeinfo, "%I:%M"); // 12 hr time, minutes
  display.setFont(&LexendMega_Regular9pt7b);
//  Serial.println(display.getCursorX()); 
  display.println(&timeinfo, "%p"); // AM or PM
  display.update();
}

// Displays icons based on OpenWeather api response
void showWeatherIcon(){
  // icon width and heights defined in weatherIcons.h
  uint16_t x_coord = 290; // Placement of icons on screen
  uint16_t y_coord = 85; // Screen dims: 296px wide, 128px tall
  
  if(iconID.equals("01")){
    display.drawBitmap(sun,(x_coord-sun_w),(y_coord-sun_h),sun_w,sun_h,GxEPD_RED); // Sun Icon
  }
  else if(iconID.equals("02")){
    display.drawBitmap(partly,(x_coord-partly_w),(y_coord-partly_h),partly_w,partly_h,GxEPD_BLACK); // Partly cloudy
    display.drawBitmap(redpartly,(x_coord-partly_w),(y_coord-partly_h),partly_w,partly_h,GxEPD_RED); // Red portion
  }
  else if(iconID.equals("03")){
    display.drawBitmap(cloud,(x_coord-cloud_w),(y_coord-cloud_h),cloud_w,cloud_h,GxEPD_BLACK); // Single cloud icon
  }
  else if(iconID.equals("04")){
    display.drawBitmap(heavyclouds,(x_coord-heavyclouds_w),(y_coord-heavyclouds_h),heavyclouds_w,heavyclouds_h,GxEPD_BLACK);
  }
  else if(iconID.equals("09")){
    display.drawBitmap(heavyrain,(x_coord-heavyrain_w),(y_coord-heavyrain_h),heavyrain_w,heavyrain_h,GxEPD_BLACK);
  }
  else if(iconID.equals("10")){
    display.drawBitmap(rain,(x_coord-rain_w),(y_coord-rain_h),rain_w,rain_h,GxEPD_BLACK); // lighter rain with sun
//    display.drawBitmap(redrain,(x_coord-rain_w),(y_coord-rain_h),rain_w,rain_h,GxEPD_RED);
  }
  else if(iconID.equals("11")){
    display.drawBitmap(thunder,(x_coord-thunder_w),(y_coord-thunder_h),thunder_w,thunder_h,GxEPD_BLACK); // thunderstorm
    display.drawBitmap(redthunder,(x_coord-thunder_w),(y_coord-thunder_h),thunder_w,thunder_h,GxEPD_RED);
  }
  else if(iconID.equals("13")){
    display.drawBitmap(snow,(x_coord-snow_w),(y_coord-snow_h),snow_w,snow_h,GxEPD_BLACK);// Snow
  }
  else if(iconID.equals("50")){
    display.drawBitmap(fog,(x_coord-fog_w),(y_coord-fog_h),fog_w,fog_h,GxEPD_BLACK);
  }
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

}
