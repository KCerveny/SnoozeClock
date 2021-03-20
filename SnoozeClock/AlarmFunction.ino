// Timer ISR: If time == alarm time, ring
void ringAlarm(){  
  // if hr, min, and date line up, playback ring
  if(timeinfo.tm_min == ringMin && timeinfo.tm_hour == ringHr && alarmDays[timeinfo.tm_wday] == 1 && alarmSet == 1){
    // ring that homie
    isRinging = true; 
    digitalWrite(12, HIGH); // May end up taking this out
  }
}

// Alarm noise
// Turn off alarm function
void alarmOff(){
  isRinging = false; 
  digitalWrite(12, LOW); // Stand-in for alarm
  ringMin = alarmMin; // Reset alarm to ring next at user-set time
  ringHr = alarmHr;
}

void snooze(){
  isRinging = false; 
  addTime();
}

// Snooze button was hit when the alarm rang; change time to next alarm
// Snooze time is ~8 mins per snooze button hit
void addTime(){
  int timeAdded = 8;
  ringHr = ringHr +( (timeinfo.tm_min + timeAdded) / 60); // Add 1 to hr if mins pushes over 60
  ringHr = (ringHr % 24); // Roll over if illegal hour value
  ringMin = (timeinfo.tm_min + timeAdded) % 60; 
}

// Alarm Setting Helper Functions
// ##############################
setAlarmHour(){
  alarmHr += scrollChange; 
  alarmHr %= 24; // If negative or greater than 23, bring back to range [0,23]
  if(ampm ^ (alarmHr < 12)) alarmHr += (alarmHr < 12) ? 12 : -12;
}

setAlarmMin(){
  alarmMin += scrollChange;
  alarmMin %= 60;
}

setAlarmAMPM(){
  if(scrollChange%2 != 0){ // If we scrolled odd number, toggle AMPM
    alarmAMPM ^= 1; // Toggle AMPM value
    alarmHr += (alarmHr < 12) ? 12 : -12; // if less than 12, add 12, else subtract 12
  }
}

setAlarmSchedule(){
  alarmSchedule += scrollChange;
  alarmSchedule %= 3; // Adjust to range [0:2]
  switch(alarmSchedule){
    case 0: // 7day
      alarmDays = {1, 1, 1, 1, 1, 1, 1};
      break;
    case 1: // wkdy
      alarmDays = {0, 1, 1, 1, 1, 1, 0};
      break;
    case 2: // wknd
      alarmDays = {1, 0, 0, 0, 0, 0, 1};
      break;
  }
}
