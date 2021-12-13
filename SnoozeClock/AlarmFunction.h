#ifndef ALARMFUNCTION_H
#define ALARMFUNCTION_H 

#include "Arduino.h"

class AlarmFunction{

  public: 

    volatile bool isRinging; // FSM button override
  
    AlarmFunction(int buzzerPin);
    void activateAlarm(struct tm timeinfo);
    void alarmSound();

    void alarmOff();
    void snooze(struct tm timeinfo);

    void toggleAlarm(){ alarmSet ^= 1; }
    void setAlarmHour(int scrollChange);
    void setAlarmMin(int scrollChange);
    void setAlarmAMPM(int scrollChange);
    void setAlarmSchedule(int scrollChange);

    bool getAlarmSetting() { return this->alarmSet; }
    int getAlarmHour() { return this->alarmHr; }
    int getAlarmMin() { return this->alarmMin; }
    int getAlarmAMPM() { return this->alarmAMPM; }
    bool* getAlarmSchedule() { return this->alarmDays; } // What is the function of the alarmSchedule variable?

  protected: 

    void addTime(struct tm timeinfo);

    int BUZZER;

    bool alarmSet; // 1=true, 0=false (We use 1,0 to store in Blynk virtual pin)
    int alarmHr;  // Hr and Min saved to servers on V6
    int alarmMin;
    int alarmAMPM; // 0=AM, 1=PM
    int alarmSchedule; // 0: 7day, 1: wkdy, 2: wknd
    
    /* Used for ringing alarm */
    int ringMin; // Used to actually ring the alarm, in case snoozed
    int ringHr;
    bool alarmDays[7]; // Index represents days since Sunday

    // Alarm Sound FSM
    volatile unsigned short buzzerState = 0;
    volatile unsigned short nextBuzzerState = 0; // Manages the buzzer FSM states
    volatile unsigned short buzzerCounter = 0; // Manages timing of the beeps

};




// // Restore alarm clock settings
// BLYNK_WRITE(V6) {
//   alarmHr = param[0].asInt();
//   alarmMin = param[1].asInt();
//   // Backup alarm day settings
//   for(int i=0; i<7; i++){
//     alarmDays[i] = param[i+2].asInt();
//   }
//   alarmSet = param[9].asInt();
// }

// // Restore last 10 messages from Blynk server
// BLYNK_WRITE(V7) {
//   Serial.println("Writing to messages backup");
//   for (int j = 0; j < MAX_MESSAGES ; j++) {
//     messages[j] = param[j].asStr();
//     Serial.println(messages[j]);
//   }
// }

AlarmFunction::AlarmFunction(int buzzerPin){

    BUZZER = buzzerPin; // Digital output pin for the alarm
    pinMode(BUZZER, OUTPUT); // TESTING LED for alarm function
    digitalWrite(BUZZER, LOW);

    // Set all instance variables
    alarmSet = 1; // 1=true, 0=false (We use 1,0 to store in Blynk virtual pin)
    alarmHr = 12;  // Hr and Min saved to servers on V6
    alarmMin = 49;
    alarmAMPM = 0; // 0=AM, 1=PM
    alarmSchedule = 0; // 0: 7day, 1: wkdy, 2: wknd
    
    /* Used for ringing alarm */
    ringMin = alarmMin; // Used to actually ring the alarm, in case snoozed
    ringHr = alarmHr;
    isRinging = false; // FSM button override
    for(int i=0; i<7; i++){
      alarmDays[i] = 1; // Index represents days since Sunday
    }
}

// Timer ISR: If time == alarm time, ring
void AlarmFunction::activateAlarm(struct tm timeinfo){  
  // if hr, min, and date line up, playback ring
  if(timeinfo.tm_min == ringMin && timeinfo.tm_hour == ringHr && alarmDays[timeinfo.tm_wday] == 1 && alarmSet == 1){
    // ring that homie

    #ifdef SERIAL_DEBUGGING
    Serial.println("Ringing the alarm");
    #endif
    
    isRinging = true; 
    digitalWrite(BUZZER, HIGH); // May end up taking this out
  }
}

// ISR: Handles the pattern of buzzing for an active alarm
// Beeps 4x in 1s, rest for 0.5s, repeat
// Assumes ISR called every ~125ms
void AlarmFunction::alarmSound(){

  // Handle when the alarm is snoozed or turned off
  if(!isRinging){
    digitalWrite(BUZZER, LOW); // Stand-in for alarm
    nextBuzzerState = 0;
    buzzerState = 0;
  }

  switch(buzzerState){

    case 0: // Standby, alarm is off until isRinging == true
      digitalWrite(BUZZER, LOW);
      if(isRinging == true){
        nextBuzzerState = 1; // we can begin ringing the alarm
        buzzerCounter = 0; // Counter is reset
      }
      else{
        nextBuzzerState = 0; // we will stay at standby
      }
      break; 

    case 1: // 4 beeps in one second

      if(buzzerCounter % 2 == 0){
        digitalWrite(BUZZER, HIGH); // alternate between on and off states
      }
      else{
        digitalWrite(BUZZER, LOW);
      }
      buzzerCounter ++; 

      if(buzzerCounter >= 7){ // transition to the rest period
        digitalWrite(BUZZER, LOW);
        buzzerCounter = 0; // Reset counter
        nextBuzzerState = 2;
      }
      break; 

    case 2: // .5s of silence
      buzzerCounter ++; // Increment, buzzer will have been turned off in state transition

      if(buzzerCounter >= 4){ // End of rest, return to buzzing
        buzzerCounter = 0; // Reset counter
        nextBuzzerState = 1;
      }
      break;

    default: // Assume the alarm is off by default
      digitalWrite(BUZZER, LOW); 
      buzzerState = 0;
      nextBuzzerState = 0;
      buzzerCounter = 0;
      break;
  }

  buzzerState = nextBuzzerState; // Transition the state
}

// Alarm noise
// Turn off alarm function
void AlarmFunction::alarmOff(){
  isRinging = false;   
  ringMin = alarmMin; // Reset alarm to ring next at user-set time
  ringHr = alarmHr;
}

void AlarmFunction::snooze(struct tm timeinfo){
  isRinging = false;
  digitalWrite(BUZZER, LOW);
  addTime(timeinfo);
}

// Snooze button was hit when the alarm rang; change time to next alarm
// Snooze time is ~8 mins per snooze button hit
void AlarmFunction::addTime(struct tm timeinfo){
  int timeAdded = 8;
  ringHr = ringHr +( (timeinfo.tm_min + timeAdded) / 60); // Add 1 to hr if mins pushes over 60
  ringHr = (ringHr % 24); // Roll over if illegal hour value
  ringMin = (timeinfo.tm_min + timeAdded) % 60; 
}

/* SET ALARM HELPER FUNCTIONS */
void AlarmFunction::setAlarmHour(int scrollChange){
  alarmHr += scrollChange; 
  alarmHr %= 24; // If negative or greater than 23, bring back to range [0,23]
  if(alarmAMPM ^ (alarmHr > 11)){
    alarmHr += alarmAMPM ? -12 : 12;
  }
}

void AlarmFunction::setAlarmMin(int scrollChange){
  alarmMin += scrollChange;
  alarmMin %= 60;
}

void AlarmFunction::setAlarmAMPM(int scrollChange){
  if(scrollChange%2 != 0){ // If we scrolled odd number, toggle AMPM
    alarmAMPM ^= 1; // Toggle AMPM value
    alarmHr += (alarmHr < 12) ? 12 : -12; // if less than 12, add 12, else subtract 12
  }
}

void AlarmFunction::setAlarmSchedule(int scrollChange){
  alarmSchedule += scrollChange;
  alarmSchedule %= 3; // Adjust to range [0:2]
  switch(alarmSchedule){
    case 0: // 7day
      for(int i=0; i<7; i++){alarmDays[i] = 1;}
      break;
    case 1: // wkdy
      alarmDays[0] = 0;
      alarmDays[1] = 1;
      alarmDays[2] = 1;
      alarmDays[3] = 1;
      alarmDays[4] = 1;
      alarmDays[5] = 1;
      alarmDays[6] = 0;
      break;
    case 2: // wknd
      alarmDays[0] = 1; // Sun
      alarmDays[1] = 0;
      alarmDays[2] = 0;
      alarmDays[3] = 0;
      alarmDays[4] = 0;
      alarmDays[5] = 0;
      alarmDays[6] = 1; // Sat
      break;
  }
}

#endif
