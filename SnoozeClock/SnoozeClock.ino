
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
char ssid[] = "ATTcIIbe6a"; // WiFi credentials.
char pass[] = "ss7aaffspp#m"; // Set password to "" for open networks.

// Messages Table
WidgetTable table;
BLYNK_ATTACH_WIDGET(table, V3);
int tableIndex; // Track position in table

// Virtual Connections/Pins
WidgetTerminal terminal(V1); // Attach virtual serial terminal to Virtual Pin V1
WidgetLED led1(V2); // Notification LED in Blynk app

// FSM Variables
short currentState; // TODO: This may actually be better as a virtual pin, to sync with Blynk servers!
short nextState;

// We make these values volatile, as they are used in interrupt context
volatile bool backChange = false;
volatile bool confirmChange = false;

BlynkTimer timer; 

// ************************ HELPER FUNCTIONS ******************************

// Sync device state with server
BLYNK_CONNECTED(){
  Blynk.syncVirtual(V5); // Messages table index updated
  
}

// Restore index counter from server
BLYNK_WRITE(V5){
  tableIndex = param.asInt(); 
}

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

// Updates clock time if minute changes
void checkTime() {

  // If checked mins != clock mins, update clock GUI
}

// FSM change based on button presses
void stateChange(){
  

  switch(currentState){ 
    case 0: // Main Clock screen
    
      // TODO: display current time GUI
      
      if(backChange && !confirmChange){
        nextState = 3; // Set alarm
        Serial.println(nextState); 
      }
      if(!backChange && confirmChange){
        nextState = 1; // Check messages
        Serial.println(nextState); 
      }
      break; 
      
    case 1: // Message Overview Screen
      
      // TODO: Show messages GUI

      digitalWrite(inbox, LOW); // Messages seen
      led1.off(); 
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock screen
        Serial.println(nextState);  
      }
      if(!backChange && confirmChange){
        nextState = 2; // Open message
        Serial.println(nextState); 
      }
      break; 
      
    case 2: // Open Message Screen

      // TODO: Open Message GUI
      
      if(backChange && !confirmChange){
        nextState = 1; // Check messages
        Serial.println(nextState); 
      }
      if(!backChange && confirmChange){
        nextState = 1; // Send response + Check messages
        Serial.println(nextState); 
      }
      break; 
      
    case 3: // Set Alarm Screen
  
      // TODO: set alarm GUI settings
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock
        Serial.println(nextState); 
      }
      if(!backChange && confirmChange){
        nextState = 0; // Confirm settings + clock
        Serial.println(nextState); 
      }
      break;
      
    default: 
      // TODO
      // We will default to the clock screen
      break;
  }

  backChange = false; 
  confirmChange = false; 
  currentState = nextState; 
}

// Process message from terminal in Blynk App
BLYNK_WRITE(V1){
  
  digitalWrite(inbox, HIGH); // Turn on the indicator lights (unread message)
  led1.on(); 

  table.addRow(tableIndex, param.asStr(), "No Response"); 
  tableIndex ++; 
  Blynk.virtualWrite(V5, tableIndex); 
    
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
//  timer.setInterval(1000L, printLocalTime); // Check clock time once per second

  
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
