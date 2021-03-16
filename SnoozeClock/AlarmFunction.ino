// Timer ISR: If time == alarm time, ring
void ringAlarm(){  
  // if hr, min, and date line up, playback ring
  if(timeinfo.tm_min == ringMin && timeinfo.tm_hour == ringHr && alarmDays[timeinfo.tm_wday] == 1 && alarmSet == 1){
    // ring that homie
    isRinging = true; 
    digitalWrite(12, HIGH); 
  }
}

// Alarm noise
// Turn off alarm function

// Snooze button was hit when the alarm rang; change time to next alarm
// Snooze time is ~8 mins per snooze button hit
void addTime(){
  int timeAdded = 8;
  ringHr = ringHr +( (timeinfo.tm_min + timeAdded) / 60); // Add 1 to hr if mins pushes over 60
  ringHr = (ringHr % 24); // Roll over if illegal hour value
  ringMin = (timeinfo.tm_min + timeAdded) % 60; 
}
