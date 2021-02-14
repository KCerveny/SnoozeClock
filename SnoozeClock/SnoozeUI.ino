#include "Fonts/LexendMega_Regular9pt7b.h"
#include "Fonts/LexendMega_Regular12pt7b.h"
#include "Fonts/LexendMega_Regular18pt7b.h"
#include "Fonts/LexendMega_Regular20pt7b.h"
#include "Fonts/LexendMega_Regular24pt7b.h"
#include "Fonts/LexendMega_Regular30pt7b.h"
#include "Fonts/LexendMega_Regular36pt7b.h"

// State 0
void clockDisplay() {
  Serial.println("clk disp");
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_RED);
  display.setFont(&LexendMega_Regular9pt7b);
  display.setCursor(0, -7);
  display.println();
  display.print(&timeinfo, "%A, %b "); // full weekday name, abreviated month name, day
  display.println(timeinfo.tm_mday % 31);

  display.setFont(&LexendMega_Regular36pt7b);
  display.setTextColor(GxEPD_BLACK);

  display.setCursor(23, 90);
  display.print(timeinfo.tm_hour % 12);
  display.print(&timeinfo, ":%M"); // 12 hr time, minutes
  display.setFont(&LexendMega_Regular9pt7b);
  Serial.print(display.getCursorX()); 
  display.println(&timeinfo, "%p");
  display.update();
}

// State 1
void messagesOverview() {

  // TODO: display message icon if new message, alarm if alarm is set :)

}

// State 2
void openMessageScreen() {

}

// ALARM SCREENS: States 3,4,5,6
// =============================

// Helper Function: set alarm screen vars
// Called to init screen, partial refresh after this
void alarmScreenInit() {

}

// State 3
void setAlarmScreen() {
  alarmScreenInit();
  // Underline? Black box around the outside? Something to indicate what we are setting
  // If alarmSet == true; on is red
  // Else, off is red
}

// State 4
void setHoursScreen() {

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
