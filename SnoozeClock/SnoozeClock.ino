#define SERIAL_DEBUGGING
#define TIMER_INTERRUPT_DEBUG         4
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include "time.h"
#include <Preferences.h>
#include <WiFiMulti.h>

#include "SimpleTimer.h"
#include "ESP32_New_TimerInterrupt.h"

#include "esp_log.h"

#ifdef CONFIG_LOG_DEFAULT_LEVEL
#undef CONFIG_LOG_DEFAULT_LEVEL
#define CONFIG_LOG_DEFAULT_LEVEL 5
#endif

#include "AlarmFunction.h"

// GPIO Pins
#define nightLight 35 // night LED to light screen
#define inbox 14 // Message received indicator LED
#define onboard 2 // System LED
#define back 22 // Alarm/back button
#define confirm 21 // Messages/forward/confirm
#define rot1 32 // Rotary dial pins
#define rot2 33
#define BUZZER 12

// Geolocation Variables
String googleApiKey = "AIzaSyATHJVr_jm1ZXj-QLr_OhEfc-v4JIYaheE";

typedef struct cl{
  String lon = "-97.7392";
  String lat = "30.2856";
  String acc = "";
  String city = "Austin";
  String country = "US"; 
  long tzSec = -18000;
  bool foundLoc = false;
} clkloc_t;

struct cl clockLocation; 

// Alarm Function variables
AlarmFunction clockAlarm(BUZZER);
AlarmFunction* alarms;

// UTP Variables
const char* ntpServer = "pool.ntp.org";
long  cstOffset_sec; // time zone
int daylightOffset_sec = 0; // daylight savings hour
struct tm timeinfo; // Holds searched time results
byte minutes; // Holds current minute value for clock display

// OpenWeather Variables
String openWeatherMapApiKey = "4428b3a249626b07f1a2769374ecf1ce";
String lon; //= "-97.7392";
String lat; //= "30.2856";
String exclude = "minutely,daily";
String jsonBuffer; 
String temp = "--"; // global store of weather as String
String precip = "--%";
String iconID; // Code for OpenWeather icon to display
bool seenAlert = false;
long sunrise; 
long sunset; 

// Connectivity Credentials
Preferences preferences;
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "SpicySchemeTeam"; // WiFi credentials.
char pass[] = "spicybrainthot"; // Set password to "" for open networks.

struct wf{
  String ssid;
  String pass; 
};
struct wf getWifi;

// Messages Variables
#define MAX_MESSAGES 10
String messages[MAX_MESSAGES]; // Keep Track of all messages sent

// FSM Variables
volatile short currentState; // TODO: This may actually be better as a virtual pin, to sync with Blynk servers!
volatile short nextState;

// ISR Variables
volatile bool backChange = false; // We make these values volatile, as they are used in interrupt context
volatile bool confirmChange = false;
volatile bool interacted = false;
volatile int rotState; // State:(rot1,rot2) {0:00},{1:01},{2:10},{3:11}
volatile int scrollChange = 0; // Zero is no scroll, negative scroll up/back, positive scroll down/forward 
volatile bool isNight = false;

#define TIMER1_INTERVAL_MS  125L
ESP32Timer ITimer1(1);
SimpleTimer timer;

// ************************ HELPER FUNCTIONS ******************************

bool checkIsNight(){
  long epochTime = mktime(&timeinfo);
  if((sunset < epochTime) || (sunrise > epochTime)){
     digitalWrite(onboard, HIGH);
     isNight = true;
     return true;
  }
  else{
    digitalWrite(onboard, LOW); // It is daytime
    isNight = false;
    return false;
  }
}

void pressBack() {
  interacted = true;
  backChange = true; // Mark pin value changed
//  if(isNight()) digitalWrite(nightLight, HIGH);
}

// Confirm switch is connected to rotary encoder
void pressConfirm() {
  interacted = true;
  confirmChange = true; // Mark pin value changed
  if(isNight){
    digitalWrite(nightLight, HIGH);
  }
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
  if(isNight) digitalWrite(nightLight, HIGH);
}

// Timer-called ISR: if no interaction since last call, return to Clock display (St. 0)
void noInteract() {
  // If no interaction in last 10 seconds, return to clock screen
  if (interacted == false && currentState != 0) {
    currentState = 0; // Adjust FSM for UI purposes
    nextState = 0;
    Serial.print("No Interaction: back to State 0");
    Serial.println(nextState);
    digitalWrite(nightLight, LOW); // Turn off light if not touched
    clockDisplay();
  }
  interacted = false; // If no buttons pushed before next call, we will return
}

// ISR to update clock time, sync with server
// Updates display if showing clock screen
void updateClock() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  if (checkIsNight() == true){
    // Change the system colors to night mode
  }
  
  if (timeinfo.tm_min != minutes && currentState == 0) {
    minutes = timeinfo.tm_min;
    clockDisplay(); // Update clock display
  }
}

void checkAlarms(){
  if(clockAlarm.checkAlarmTime(timeinfo) == true){
    clockAlarm.activateAlarm();
  }
}

//// Handles the noise of the alarm buzzer
// uses hardware ISR so not blocked by long updates/refresh cycle
bool IRAM_ATTR soundAlarm(void * timerNo){
  if(clockAlarm.isRinging == true){
    clockAlarm.alarmSound();
  }
  
  return true;
}

// ************************************* MAIN DRIVER FUNCTIONS **********************************

void setup() {
  Serial.begin(115200);

  initDisplay();

  esp_log_level_set("*", ESP_LOG_VERBOSE);

//  wifiProvision();
  getWifi.ssid = ssid; 
  getWifi.pass = pass;

  /* Init Wireless and Location Services */
//  systemBootScreen();
//  getCoords(); // Calls Google Geolocation API
//  if(clockLocation.foundLoc == false){
//    getCoords; // Attempt once more before default location
//  }

  WiFi.mode(WIFI_STA);
  WiFi.begin((getWifi.ssid).c_str(), (getWifi.pass).c_str());
  Serial.println("Connecting to WiFi: " + getWifi.ssid);
  while (WiFi.status() != WL_CONNECTED);
  
  locationStatus();
  configTime(clockLocation.tzSec, 0, ntpServer); //init and get the time
  getLocalTime(&timeinfo); 
  getWeather();
  /* End Location Setup */
  sysBootStatusScreen();

  pinMode(inbox, OUTPUT); // Notification light
//  digitalWrite(inbox, LOW);
  pinMode(onboard, OUTPUT);
  digitalWrite(onboard, LOW);
  pinMode(nightLight, OUTPUT);
  digitalWrite(nightLight, LOW);
  
  pinMode(back, INPUT);
  pinMode(confirm, INPUT);
  pinMode(rot1, INPUT);
  pinMode(rot2, INPUT);
  delay(50); // allow rotary pins to debounce
  int rotary1 = digitalRead(rot1) ? 1 : 0; // If HIGH, make 1, else 0
  int rotary2 = digitalRead(rot2) ? 1 : 0;
  rotState = 2*rotary1 + rotary2; // Will yield 4 distinct states


  currentState = 0; // Initialize FSM of clock interface
  nextState = 0;

  // Attach back/confirm button interrupts to our handler
  attachInterrupt(digitalPinToInterrupt(back), pressBack, FALLING); // TODO: add interrupt service routine for button presses
  attachInterrupt(digitalPinToInterrupt(confirm), pressConfirm, FALLING); // State changes when the button is released
  attachInterrupt(digitalPinToInterrupt(rot1), scrollWheel, CHANGE); // Rotary dial lead 1
  attachInterrupt(digitalPinToInterrupt(rot2), scrollWheel, CHANGE); // Rotary dial lead 2

  // SOFTWARE TIMER INTERRUPTS (BLOCKING)
//  timer.setInterval(125L, clockAlarm.alarmSound); // Handle the alarm buzzing sound
  timer.setInterval(200L, stateChange); // State Change check function
  timer.setInterval(1000L, updateClock); // Check clock time once per second
  timer.setInterval(10000L, noInteract); // If no interactions for 10 seconds, go back to clock (S0)
  timer.setInterval(20000L, checkAlarms); // Each 20 secs, check if alarm needs to ring
  timer.setInterval(300000L, getWeather); // Retrieve current weather every 5 mins

//  // HARDWARE ISR
  ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, soundAlarm); // Check for alarm sound 125ms, not blocked by other functions

  clockDisplay(); // Set first screen to the clock (home) screen

}

void loop() {
  timer.run(); // Handle software interrupts  
}
