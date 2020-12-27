
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

#define inbox 23 // Message received indicator LED
#define back 22 // Alarm/back button
#define confirm 21 // Messages/forward/confirm

// UTP Variables
const char* ntpServer = "pool.ntp.org";
const long  cstOffset_sec = -21600;
const int   daylightOffset_sec = 3600;

// Connectivity Credentials
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "ATTcIIbe6a"; // Your WiFi credentials.
char pass[] = "ss7aaffspp#m"; // Set password to "" for open networks.

// Virtual Connections/Pins
WidgetTerminal terminal(V1); // Attach virtual serial terminal to Virtual Pin V1
WidgetLED led1(V2); // Notification LED in Blynk app

// FSM Variables
short currentState; // TODO: This may actually be better as a virtual pin, to sync with Blynk servers!
short nextState;

// We make these values volatile, as they are used in interrupt context
volatile bool backChange = false;
volatile bool confirmChange = false; 
bool bkCh = false; 
bool cfCh = false; 

BlynkTimer timer; 

// ************************ HELPER FUNCTIONS ******************************

void pressBack(){
//  backValue = digitalRead(back);
  backChange = true; // Mark pin value changed
}

void pressConfirm(){
//  confirmValue = !digitalRead(confirm);// Invert state, since button is "Active LOW"
  confirmChange = true; // Mark pin value changed
}

// Accesses and prints CST time from UTP server
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  terminal.println(&timeinfo, "%A, %B %d %Y %H:%M");
}

// FSM change based on button presses
void stateChange(){
  

  switch(currentState){ 
    case 0: // Main Clock screen
    
      // TODO: display current time GUI
      terminal.println("Main Clock Screen"); 
      
      if(backChange && !confirmChange){
        nextState = 3; // Set alarm
      }
      if(!backChange && confirmChange){
        nextState = 1; // Check messages
      }
      break; 
      
    case 1: // Message Overview Screen
      
      // TODO: Show messages GUI
      terminal.println("Messages Over. Screen"); 

      digitalWrite(inbox, LOW); // Messages seen
      led1.off(); 
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock screen
      }
      if(!backChange && confirmChange){
        nextState = 2; // Open message
      }
      break; 
      
    case 2: // Open Message Screen

      // TODO: Open Message GUI
      terminal.println("Open Mess. Screen"); 
      
      if(backChange && !confirmChange){
        nextState = 1; // Check messages
      }
      if(!backChange && confirmChange){
        nextState = 1; // Send response + Check messages
      }
      break; 
      
    case 3: // Set Alarm Screen
  
      // TODO: set alarm GUI settings
      terminal.println("Alarm Screen"); 
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock
      }
      if(!backChange && confirmChange){
        nextState = 0; // Confirm settings + clock
      }
      break;
      
    default: 
      // TODO
      // We will default to the clock screen
      break;
  }
  
  currentState = nextState; 
}

// Process message from terminal in Blynk App
BLYNK_WRITE(V1)
{
  digitalWrite(inbox, HIGH); // Turn on the indicator lights
  led1.on(); 

  // Add to the messages buffer (?)
  // Read back messages in order on BTN1 press
  // OPTION: if a blynk switch is on, requires response (pizza party?), else, just a message (Love you!)
  
}


// ************************************* MAIN DRIVER FUNCTIONS **********************************

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  configTime(cstOffset_sec, daylightOffset_sec, ntpServer); //init and get the time
  
  pinMode(inbox, OUTPUT); // Notification light
  digitalWrite(inbox, LOW); 
  led1.off();
  pinMode(back, INPUT); 
  pinMode(confirm, INPUT); 

  currentState = 0; 
  nextState = 0; 

  // Attach back/confirm button interrupts to our handler
  attachInterrupt(digitalPinToInterrupt(back), pressBack, FALLING); // TODO: add interrupt service routine for button presses
  attachInterrupt(digitalPinToInterrupt(confirm), pressConfirm, FALLING); // State changes when the button is released

  timer.setInterval(200L, stateChange); // State Change check function
  timer.setInterval(1000L, printLocalTime); // Check clock time once per second

  
  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.clear();
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  printLocalTime(); 
  terminal.println(F("-----------------"));
  terminal.flush();
}

void loop()
{
  Blynk.run();
  timer.run();
}
