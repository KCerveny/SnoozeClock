#define BLYNK_PRINT Serial /* TODO: delete this line to increase processing */
#define BLYNK_HEARTBEAT 30

#include <SPI.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include <Arduino_JSON.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

// DISPLAY LIBS
#include <GxEPD.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h> // 2.9" b/w/r
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

// GPIO Pins
#define nightLight 35 // night LED to light screen
#define inbox 23 // Message received indicator LED
#define back 22 // Alarm/back button
#define confirm 21 // Messages/forward/confirm
#define rot1 32 // Rotary dial pins
#define rot2 33

// EInk Object initialized
GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 

// UTP Variables
const char* ntpServer = "pool.ntp.org";
const long  cstOffset_sec = -21600; // time zone
const int   daylightOffset_sec = 3600; // daylight savings hour
struct tm timeinfo; // Holds searched time results
byte minutes; // Holds current minute value for clock display
long epochTime; 

// OpenWeather Variables
String openWeatherMapApiKey = "4428b3a249626b07f1a2769374ecf1ce";
String cityID = "4671654"; // Austin
String lon = "-97.7431";
String lat = "30.2672";
String exclude = "minutely,daily";
String jsonBuffer; 
String temp = "--"; // global store of weather as String
String precip = "--%";
String iconID; // Code for OpenWeather icon to display
bool seenAlert = false;
long sunrise; 
long sunset; 

// Alarm Variables
TaskHandle_t Task1; // freeRTOS task kernel variable

/* Used for setting alarm values */
bool alarmSet = 1; // 1=true, 0=false (We use 1,0 to store in Blynk virtual pin)
int alarmHr = 7;  // Hr and Min saved to servers on V6
int alarmMin = 30;
int alarmAMPM = 0; // 0=AM, 1=PM
int alarmSchedule = 0; // 0: 7day, 1: wkdy, 2: wknd

/* Used for ringing alarm */
int ringMin; // Used to actually ring the alarm, in case snoozed
int ringHr;
bool alarmDays[7] = {1, 1, 1, 1, 1, 1, 1}; // Index represents days since Sunday
volatile bool isRinging; // FSM button override

// Connectivity Credentials
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "SpicySchemeTeam"; // WiFi credentials.
char pass[] = "spicybrainthot"; // Set password to "" for open networks.

// Messages Variables
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
  checkNightLight();
}

// Confirm switch is connected to rotary encoder
void pressConfirm() {
  interacted = true;
  confirmChange = true; // Mark pin value changed
  checkNightLight();
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
  checkNightLight();
}

void checkNightLight(){
  long offset = 86400; // 60*60*24 approx. seconds in a day
  // if current time is between sunset and sunrise of coming day, turn the screen light on
  if((sunset < epochTime) && ((sunrise + offset) > epochTime)){
     digitalWrite(nightLight, HIGH);
  }
}

// Timer-called ISR: if no interaction since last call, return to Clock display (St. 0)
void noInteract() {
  // If no interaction in last 10 seconds, return to clock screen
  if (interacted == false && currentState != 0) {
    currentState = 0; // Adjust FSM for UI purposes
    nextState = 0;
    Serial.print("Back to 0: ");
    Serial.println(nextState);
    digitalWrite(nightLight, LOW); // Turn off light if not touched
    clockDisplay();
  }
  interacted = false; // If no buttons pushed before next call, we will return
}

// ISR to update clock time, sync with server
// Updates display if showing clock screen
void updateClock() {
  time(&epochTime); // Get Unix time in seconds
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
  String serverPath = "https://api.openweathermap.org/data/2.5/onecall?lat="+lat+"&lon="+lon+"&units=imperial&appid="+openWeatherMapApiKey;
  jsonBuffer = httpGETRequest(serverPath.c_str());  
  JSONVar myObject = JSON.parse(jsonBuffer);
  
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    temp = "--";
    precip = "--";
    return;
  }
  double tempRead = round(double(myObject["current"]["temp"]));
  temp = String(int(tempRead)); // Round, truncate decimal, convert to string
  double precipRead = round(double(myObject["hourly"][0]["pop"])); 
  precip = String(int(precipRead));
  iconID = (JSON.stringify(myObject["hourly"][0]["weather"][0]["icon"])).substring(1,3); // Get ID of weather icon
  Serial.println("ICON: " + iconID);
  sunrise = long(myObject["sys"]["sunrise"]);
  sunset = long(myObject["sys"]["sunset"]);

  // National weather alerts and warnings
  if(myObject.hasOwnProperty("alert") && !seenAlert){
    seenAlert = true;
    Serial.println("National Weather Alert!");
    String alert = JSON.stringify(myObject["alert"]["sender_name"]) +": "+ JSON.stringify(myObject["alert"]["event"]);
    addMessage(alert); // Weather alert will show up in inbox
  }
  else if(!myObject.hasOwnProperty("alert") && seenAlert){
    seenAlert = false; // We can set flag to false now that the alert is over
  }
}

// ************************************* MAIN DRIVER FUNCTIONS **********************************

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH); // Smooth readjustment of system time with NTP
  configTime(cstOffset_sec, daylightOffset_sec, ntpServer); //init and get the time
  getLocalTime(&timeinfo); 
  
  getWeather();

  pinMode(inbox, OUTPUT); // Notification light
  digitalWrite(inbox, LOW);
  pinMode(nightLight, OUTPUT);
  digitalWrite(nightLight, LOW);
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

  Serial.print("Setup on core ");
  Serial.println(xPortGetCoreID());

  /* SECOND CORE RUNS SOUND TASKS */
//  xTaskCreatePinnedToCore(
//                    alarmSound,   /* Task function. */
//                    "alarmSound",     /* name of task. */
//                    10000,       /* Stack size of task */
//                    NULL,        /* parameter of the task */
//                    1,           /* priority of the task */
//                    &Task1,      /* Task handle to keep track of created task */
//                    0);          /* pin task to core 0 */
//  delay(500);

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
