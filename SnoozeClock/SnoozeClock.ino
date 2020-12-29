
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

// GUI Libraries
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

#define inbox 23 // Message received indicator LED
#define back 22 // Alarm/back button
#define confirm 21 // Messages/forward/confirm

// DISPLAY  VARIABLES

#define TFT_CS        5
#define TFT_RST       4 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// color definitions
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;

// The colors we actually want to use
uint16_t        Display_Text_Color         = Display_Color_Black;
uint16_t        Display_Backround_Color    = Display_Color_Blue;

// assume the display is off until configured in setup()
bool            isDisplayVisible        = false;

// declare size of working string buffers. Basic strlen("d hh:mm:ss") = 10
const size_t    MaxString               = 16;

// the string being displayed on the SSD1331 (initially empty)
char oldTimeString[MaxString]           = { 0 };

// END DISPLAY VARIABLES


// UTP Variables
const char* ntpServer = "pool.ntp.org";
const long  cstOffset_sec = -21600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo; // Holds searched time results
byte minutes; // Holds current minute value

// Connectivity Credentials
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "ATTcIIbe6a"; // WiFi credentials.
char pass[] = "ss7aaffspp#m"; // Set password to "" for open networks.

// Messages Table
WidgetTable table;
BLYNK_ATTACH_WIDGET(table, V3);
int tableIndex; // Track position in table
String Messages[10]; // Keep Track of all messages sent


// Virtual Connections/Pins
WidgetTerminal terminal(V1); // Attach virtual serial terminal to Virtual Pin V1
WidgetLED led1(V2); // Notification LED in Blynk app

// FSM Variables
short currentState; // TODO: This may actually be better as a virtual pin, to sync with Blynk servers!
short nextState;

// We make these values volatile, as they are used in interrupt context
volatile bool backChange = false;
volatile bool confirmChange = false;
volatile bool interacted = false; 

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
  interacted = true;
  backChange = true; // Mark pin value changed
}

void pressConfirm(){
  interacted = true; 
  confirmChange = true; // Mark pin value changed
}

void noInteract() {
  // If no interaction in last 10 seconds, return to clock screen
  if(interacted == false){
    currentState = 0; // Adjust FSM for UI purposes 
    clockDisplay();
  }
  interacted = false; // If no buttons pushed before next call, we will return
}

// ISR to update clock time, sync with server
// Updates display if showing clock screen
void updateClock() {

  // TODO: update clock
  /* if connected: get server time
    else: internal time (?)

    if current mins != mins, update display
  */
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  if (timeinfo.tm_min != minutes && currentState == 0){
    minutes = timeinfo.tm_min;
    clockDisplay(); // Update clock display
  }
  
}

// Accesses and prints CST time from UTP server
void printLocalTime()
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  terminal.println(&timeinfo, "%A, %B %d %Y %H:%M");
}


// Process message from terminal in Blynk App
BLYNK_WRITE(V1){
  
  digitalWrite(inbox, HIGH); // Turn on the indicator lights (unread message)
  led1.on(); 

  table.addRow(tableIndex, param.asStr(), "No Response"); 
  Messages[tableIndex] = param.asStr();
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
  timer.setInterval(1000L, updateClock); // Check clock time once per second
  timer.setInterval(10000L, noInteract); // If no interactions for 10 seconds, go back to clock

  /* DISPLAY INITIALIZING */
  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab

  // initialise the display
  tft.setFont();
  tft.fillScreen(Display_Backround_Color);
  tft.setTextColor(Display_Text_Color);
  tft.setTextSize(1);

  // the display is now on
  isDisplayVisible = true;

  clockDisplay();
  
  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.clear();
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-----------------"));
  terminal.flush();
  
}

void loop()
{
  Blynk.run();
  timer.run();
}
