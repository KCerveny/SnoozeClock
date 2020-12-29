// Timer ISR: If time == alarm time, ring
void ringAlarm(){
  
  // if hr, min, and date line up, playback ring
  if(timeinfo.tm_min == alarmMin && timeinfo.tm_hour == alarmHr && alarmDays[timeinfo.tm_wday] == true){
    // ring that homie
    terminal.print("Ring ring!"); 
  }
}


// Use rotary dial to select hr, min, and schedule
void setAlarm() {

  Serial.print("Set alarm"); 
  
}
