// FSM change based on button presses
void stateChange(){
  
  // Button override to turn off alarm
  if(isRinging){
    if(backChange && !confirmChange) snooze(); // back button : snooze for n mins
    if(!backChange && confirmChange) alarmOff(); // confirm: turn off alarm
    
    backChange = false; 
    confirmChange = false; 
    scrollChange = 0;
    return; 
  }


  // Continue to conventional screen change
  switch(currentState){ 
    case 0: // Main Clock screen      
      if(backChange && !confirmChange){
        nextState = 3; // Set alarm
        Serial.println(nextState); 
        setAlarmScreen(); 
      }
      if(!backChange && confirmChange){
        nextState = 1; // Check messages
        Serial.println(nextState); 
        messagesOverview();
      }
      break; 
      
    case 1: // Message Overview Screen
      
      // TODO: Show messages GUI

      digitalWrite(inbox, LOW); // Messages seen
      led1.off(); 
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock screen
        Serial.println(nextState);
        clockDisplay();  
      }
      if(!backChange && confirmChange){
        nextState = 2; // Open message
        Serial.println(nextState); 
        openMessageScreen();
      }
      break; 
      
    case 2: // Open Message Screen

      // TODO: Open Message GUI
      
      if(backChange && !confirmChange){
        nextState = 1; // Check messages
        Serial.println(nextState); 
        messagesOverview();
      }
      if(!backChange && confirmChange){
        nextState = 1; // Send response + Check messages
        Serial.println(nextState); 
        messagesOverview();
      }
      break; 

    // ALARM SETTINGS: States 3,4,5,6,7
    case 3: // Alarm Enable Screen
      // Rotary dial turns alarm on/off
      if(abs(scrollChange)%2 != 0){
        alarmSet ^= 1; // If scroll is odd, alarm state switches
      }
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock
        Serial.println(nextState); 
        clockDisplay();
      }
      if(!backChange && confirmChange){
        nextState = 4; // Enabled/Disabled, proceed to hour
        Serial.println(nextState); 
      }
      break;

    case 4: // Hour Set Function
      // Rotary selection
      alarmHr += scroll
      
      if(backChange && !confirmChange){
        nextState = 3; // Return to Enable      
      }
      if(!backChange && confirmChange){
        nextState = 5; // Set minutes next
      }
      break; 

    case 5: // Minute Set Function
      // Minute selection functions
      /* Increment Minute
       * If scrollUp && !scrollDown: 
       *  alarmMin += (alarmMin + 1)%60 // Roll over back to xx:00
       *  update GUI to show new min
       */

       /* Decrement Minute
        *  If !scrollUp && scrollDown: 
        *   alarmMin --; 
        *   if(alarmMin < 0) alarmMin = 59; // roll around back to xx:59
        *   update GUI to show new min
        */
      if(backChange && !confirmChange){
        nextState = 4; // back to set hours
//        setHoursScreen(); 
      }
      if(!backChange && confirmChange){
        nextState = 6; // Set schedule next
//        setScheduleScreen(); 
      }
      break; 

    case 6: // Alarm Schedule Function
      // Schedule Select Functions

      // TODO: how to select schedules
      
      if(backChange && !confirmChange){
        nextState = 5; // Back to clock screen
//        setMinutesScreen(); 
      }
      if(!backChange && confirmChange){
        nextState = 0; // back to set minutes
        Blynk.virtualWrite(V6, alarmHr, alarmMin); // Save values to the blynk Server 
        clockDisplay(); 
      }
      break; 
    
    default: 
      // TODO
      // We will default to the clock screen
      break;
  }

  // TODO: scrollUp, scrollDown = false, have been handled in state
  scrollChange = 0;
  backChange = false; 
  confirmChange = false; 
  
  currentState = nextState; 
}
