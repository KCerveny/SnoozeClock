#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>




void displayUpTime() {
    unsigned long upSeconds = millis() / 1000; // calculate seconds, truncated to the nearest whole second   
    unsigned long days = upSeconds / 86400; // calculate days, truncated to nearest whole day   
    upSeconds = upSeconds % 86400; // the remaining hhmmss are   
    unsigned long hours = upSeconds / 3600; // calculate hours, truncated to the nearest whole hour   
    upSeconds = upSeconds % 3600; // the remaining mmss are  
    unsigned long minutes = upSeconds / 60; // calculate minutes, truncated to the nearest whole minute  
    upSeconds = upSeconds % 60; // the remaining ss are  
    char newTimeString[MaxString] = { 0 }; // allocate a buffer
    // construct the string representation
    sprintf(
        newTimeString,
        "%lu %02lu:%02lu:%02lu",
        days, hours, minutes, upSeconds
    );

    // has the time string changed since the last tft update?
    if (strcmp(newTimeString,oldTimeString) != 0) {

        // yes! home the cursor
        tft.setCursor(0,0);
        // change the text color to the background color
        tft.setTextColor(Display_Backround_Color);
        // redraw the old value to erase
        tft.print(oldTimeString);
        // home the cursor
        tft.setCursor(0,0);     
        // change the text color to foreground color
        tft.setTextColor(Display_Text_Color);    
        // draw the new time value
        tft.print(newTimeString);    
        // and remember the new value
        strcpy(oldTimeString,newTimeString);
    }
}

void clockDisplay(){
  tft.fillScreen(Display_Color_Yellow);
  tft.setCursor(1,1);
  tft.print("Clock Screen: State 0"); 
  tft.setCursor(1,13); 
  tft.print(&timeinfo, "%H:%M"); 
}

void messagesOverview() {
  tft.fillScreen(Display_Color_Green);
  tft.setCursor(1,1);
  tft.print("Clock Screen: State 1"); 

  // print out all messages
  for(int i = 0; i< tableIndex; i++){
    tft.setCursor(1, 10*(i+1)); 
    tft.print(Messages[i]); 
  }
}
