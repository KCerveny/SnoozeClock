
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

#define back 22 // Alarm/back button
#define confirm 21 // Messages/forward/confirm

// DISPLAY  VARIABLES
#include <GxEPD.h>
#include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
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

// Alarm Variables
bool alarmDays[7] = {true, true, true, true, true, true, true}; ; // TODO: may be better as virtual pin for backups
int alarmHr = 18;  // Hr and Min saved to servers on V6
int alarmMin = 55 ;
bool alarmSet = true;
int ringMin; // Used to actually ring the alarm, in case snoozed
int ringHr;
volatile bool isRinging; // FSM button override

// Connectivity Credentials
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "SpicySchemeTeam"; // WiFi credentials.
char pass[] = "spicybrainthot"; // Set password to "" for open networks.

// Messages Variables
#define inbox 23 // Message received indicator LED
#define MAX_MESSAGES 10
String messages[MAX_MESSAGES]; // Keep Track of all messages sent

/* Virtual Connections/Pins
    V0:
    V1: Terminal Input
    V2: Notification LED
    V3:
    V5:
    V6: {AlarmHr, AlarmMin}
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

void pressConfirm() {
  interacted = true;
  confirmChange = true; // Mark pin value changed
}

void scrollWheel() {
  // TODO: catch scroll wheel state, determine wether we are going "up" or "down"
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

// ************************************* MAIN DRIVER FUNCTIONS **********************************

void setup() {

  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  configTime(cstOffset_sec, daylightOffset_sec, ntpServer); //init and get the time

  pinMode(inbox, OUTPUT); // Notification light
  digitalWrite(inbox, LOW);
  led1.off();
  pinMode(back, INPUT);
  pinMode(confirm, INPUT);

  pinMode(12, OUTPUT); // TESTING LED for alarm function
  digitalWrite(12, LOW);
  ringMin = alarmMin; // Set alarm to ring next at user-set time
  ringHr = alarmHr;

  currentState = 0;
  nextState = 0;

  // Attach back/confirm button interrupts to our handler
  attachInterrupt(digitalPinToInterrupt(back), pressBack, FALLING); // TODO: add interrupt service routine for button presses
  attachInterrupt(digitalPinToInterrupt(confirm), pressConfirm, FALLING); // State changes when the button is released

  timer.setInterval(200L, stateChange); // State Change check function
  timer.setInterval(1000L, updateClock); // Check clock time once per second
  timer.setInterval(10000L, noInteract); // If no interactions for 10 seconds, go back to clock (S0)
  timer.setInterval(20000L, ringAlarm); // Each 20 secs, check if alarm needs to ring

  /* DISPLAY INITIALIZING */
  display.init(115200); // enable diagnostic output on Serial
  display.setRotation(1);
  clockDisplay();

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
