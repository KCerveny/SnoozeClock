// Timer ISR: If time == alarm time, ring
void ringAlarm(){  
  // if hr, min, and date line up, playback ring
  if(timeinfo.tm_min == ringMin && timeinfo.tm_hour == ringHr && alarmDays[timeinfo.tm_wday] == true){
    // ring that homie
    isRinging = true; 
    digitalWrite(12, HIGH); 
  }
}

// Snooze button was hit when the alarm rang; change time to next alarm
void addTime(){
  int timeAdded = 1; 
  Serial.println(ringHr); 
  ringHr = ringHr +( (timeinfo.tm_min + timeAdded) / 60); // Add 1 to hr if mins pushes over 60
  ringHr = (ringHr % 24); // Roll over if illegal hour value
  ringMin = (timeinfo.tm_min + timeAdded) % 60; 
  Serial.println("Hours, minutes"); 
  Serial.print(ringHr); 
  Serial.print(" : "); 
  Serial.println(ringMin); 
}

// Use rotary dial to select hr, min, and schedule
void setAlarm() {
  Serial.print("Set alarm"); 

  int setValue = 0; 
  int nextValue = 0; 
  bool completed = false; 
  // Navigate through options with <> buttons
  while(!completed){
    switch(setValue){
      case 0: // off/on
        // Start on off, switch to on?
        // if back: return to clock screen
        // forward: case = 1
        if(!backChange && confirmChange){
          Serial.println("Change the hours now!"); 
          nextValue = 1; 
        }
        break; 
      case 1: // hour
        // back: 0
        // forward: 2
        if(backChange && !confirmChange){
          Serial.println("exiting loop"); 
          completed = true; 
        }
        break; 
      case 2: // minute
        completed = true; // DEBUGGING ONLY
        // back: 1
        // forward: 3
        break; 
      case 3: // schedule
        // back: 2
        // forward: clock screen w/ updated values
        break; 
      default: // off/on
        // TODO: match case 0 properties
        break; 
    }
    backChange = false; 
    confirmChange = false; 
    setValue = nextValue;
    
  }
  Serial.println("Exiting set alarm function"); 
}
