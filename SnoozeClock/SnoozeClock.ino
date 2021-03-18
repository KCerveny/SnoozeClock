
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

// GUI Libraries
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include <Arduino_JSON.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

#define back 22 // Alarm/back button
#define confirm 21 // Messages/forward/confirm
#define rot1 32 // Rotary dial pin
#define rot2 33

// DISPLAY  VARIABLES
#include <GxEPD.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h> // 2.9" b/w/r
#include GxEPD_BitmapExamples // Might not be needed
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4
// END DISPLAY VARIABLES

// UTP Variables
const char* ntpServer = "pool.ntp.org";
const long  cstOffset_sec = -21600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo; // Holds searched time results
byte minutes; // Holds current minute value for clock display

// OpenWeather Variables
String openWeatherMapApiKey = "4428b3a249626b07f1a2769374ecf1ce";
String cityID = "4671654"; // Austin
String units = "imperial";
String jsonBuffer; 
String temp = "--"; // global store of weather as String
String iconID; // Code for OpenWeather icon to display

// Alarm Variables
bool alarmDays[7] = {1, 1, 1, 1, 1, 1, 1}; // Index represents days since Sunday
int alarmHr = 7;  // Hr and Min saved to servers on V6
int alarmMin = 30;
bool alarmSet = 1; // 1=true, 0=false
int ringMin; // Used to actually ring the alarm, in case snoozed
int ringHr;
volatile bool isRinging; // FSM button override

// Connectivity Credentials
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "ATTcIIbe6a"; // WiFi credentials.
char pass[] = "ss7aaffspp#m"; // Set password to "" for open networks.

// Messages Variables
#define inbox 23 // Message received indicator LED
#define MAX_MESSAGES 10
String messages[MAX_MESSAGES]; // Keep Track of all messages sent

/* Virtual Connections/Pins
    V0:
    V1: Terminal Input (Smartphone)
    V2: Notification LED
    V3:
    V5:
    V6: {AlarmHr, AlarmMin, alarmDays[0:6], alarmSet}
    V7: Sent messages storage
*/
WidgetTerminal terminal(V1); // Attach virtual serial terminal to Virtual Pin V1
WidgetLED led1(V2); // Notification LED in Blynk app

// FSM Variables
volatile short currentState; // TODO: This may actually be better as a virtual pin, to sync with Blynk servers!
volatile short nextState;

// ISR Variables
volatile bool backChange = false; // We make these values volatile, as they are used in interrupt context
volatile bool confirmChange = false;
volatile bool interacted = false;
volatile int rotState; // State:(rot1,rot2) {0:00},{1:01},{2:10},{3:11}
volatile int scrollChange = 0; // Zero is no scroll, negative scroll up/back, positive scroll down/forward 

BlynkTimer timer;

// ************************ HELPER FUNCTIONS ******************************

// Sync device state with server
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V6, V7);
}

// Restore alarm clock settings
BLYNK_WRITE(V6) {
  alarmHr = param[0].asInt();
  alarmMin = param[1].asInt();
  // Backup alarm day settings
  for(int i=0; i<7; i++){
    alarmDays[i] = param[i+2].asInt();
  }
  alarmSet = param[9].asInt();
}

// Restore last 10 messages from Blynk server
BLYNK_WRITE(V7) {
  Serial.println("Writing to messages backup");
  for (int j = 0; j < MAX_MESSAGES ; j++) {
    messages[j] = param[j].asStr();
    Serial.println(messages[j]);
  }
}

void pressBack() {
  interacted = true;
  backChange = true; // Mark pin value changed
}

// Confirm switch is connected to rotary encoder
void pressConfirm() {
  interacted = true;
  confirmChange = true; // Mark pin value changed
}

void scrollWheel() {
  // Read the current state
  int newState;
  int rotary1 = digitalRead(rot1) ? 1 : 0; // If HIGH, make 1, else 0
  int rotary2 = digitalRead(rot2) ? 1 : 0;
  newState = 2*rotary1 + rotary2; // Will yield 4 distinct states

  // TODO: need to determine order of states scrolling CC vs CCW
  switch(rotState){
    case 0: // Can go to 2 or 1
      scrollChange += ((newState == 2) ? 1 : -1);
      break; 
    
    case 1: // Can go to 0 or 3
      scrollChange += ((newState == 0) ? 1 : -1);
      break; 
    
    case 2: // Can go to 3 or 0
      scrollChange += ((newState == 3) ? 1 : -1);
      break;
      
    case 3: // Can go to 1 or 2
      scrollChange += ((newState == 1) ? 1 : -1); 
      break;
      
    default: 
      rotState = newState; // If we get here, rotState was not initialized. Ammend.
      break;
  }
  
  rotState = newState; // scrollChange has been adjusted, state updates  
}

// Timer-called ISR: if no interaction since last call, return to Clock display (St. 0)
void noInteract() {
  // If no interaction in last 10 seconds, return to clock screen
  if (interacted == false && currentState != 0) {
    currentState = 0; // Adjust FSM for UI purposes
    nextState = 0;
    Serial.print("Back to 0: ");
    Serial.println(nextState);
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
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  if (timeinfo.tm_min != minutes && currentState == 0) {
    minutes = timeinfo.tm_min;
    clockDisplay(); // Update clock display
  }
}

// ISR: OpenWeather HTTP request
// Sends HTTP request, parses JSON, stores relevant weather info
void getWeather(){
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?id=" + cityID + "&appid=" + openWeatherMapApiKey+"&units="+units;
  jsonBuffer = httpGETRequest(serverPath.c_str());
  Serial.println(jsonBuffer);
  
  JSONVar myObject = JSON.parse(jsonBuffer);
  
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    temp = "--";
    return;
  }
  int tempRead = int(myObject["main"]["temp"]);
  temp = String(tempRead); // Round, truncate decimal, convert to string
  iconID = myObject["weather"]["icon"]; 
}

// ************************************* MAIN DRIVER FUNCTIONS **********************************

void setup() {

  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  configTime(cstOffset_sec, daylightOffset_sec, ntpServer); //init and get the time
  getLocalTime(&timeinfo); 
  getWeather();

  pinMode(inbox, OUTPUT); // Notification light
  digitalWrite(inbox, LOW);
  led1.off();
  pinMode(back, INPUT);
  pinMode(confirm, INPUT);
  pinMode(rot1, INPUT);
  pinMode(rot2, INPUT);
  delay(50); // allow rotary pins to debounce
  int rotary1 = digitalRead(rot1) ? 1 : 0; // If HIGH, make 1, else 0
  int rotary2 = digitalRead(rot2) ? 1 : 0;
  rotState = 2*rotary1 + rotary2; // Will yield 4 distinct states

  pinMode(12, OUTPUT); // TESTING LED for alarm function
  digitalWrite(12, LOW);
  ringMin = alarmMin; // Set alarm to ring next at user-set time
  ringHr = alarmHr;

  currentState = 0; // Initialize FSM of clock interface
  nextState = 0;

  // Attach back/confirm button interrupts to our handler
  attachInterrupt(digitalPinToInterrupt(back), pressBack, FALLING); // TODO: add interrupt service routine for button presses
  attachInterrupt(digitalPinToInterrupt(confirm), pressConfirm, FALLING); // State changes when the button is released
  attachInterrupt(digitalPinToInterrupt(rot1), scrollWheel, CHANGE); // Rotary dial lead 1
  attachInterrupt(digitalPinToInterrupt(rot2), scrollWheel, CHANGE); // Rotary dial lead 2

  timer.setInterval(200L, stateChange); // State Change check function
  timer.setInterval(1000L, updateClock); // Check clock time once per second
  timer.setInterval(10000L, noInteract); // If no interactions for 10 seconds, go back to clock (S0)
  timer.setInterval(20000L, ringAlarm); // Each 20 secs, check if alarm needs to ring
  timer.setInterval(300000L, getWeather); // Retrieve current weather every 5 mins

  /* DISPLAY INITIALIZING */
  display.init(115200); // enable diagnostic output on Serial
  display.setRotation(1);
  clockDisplay();
  delay(2000);

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.clear();
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-----------------"));
  terminal.println(F("Previous Messages: "));
  for (int b = 0; b < MAX_MESSAGES; b++) {
    terminal.println(messages[b]);
  }
  terminal.println(F("-----------------"));
  terminal.flush();

}

void loop() {
  Blynk.run();
  timer.run();
}
