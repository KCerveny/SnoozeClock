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
      setAlarmHour();
      
      if(backChange && !confirmChange){
        nextState = 3; // Return to Enable      
      }
      if(!backChange && confirmChange){
        nextState = 5; // Set minutes next
      }
      break; 

    case 5: // Minute Set Function
      setAlarmMin();
      
      if(backChange && !confirmChange){
        nextState = 4; // back to set hours 
      }
      if(!backChange && confirmChange){
        nextState = 6; // Set AMPM next
      }
      break; 

    case 6: // AMPM Function
      setAlarmAMPM();
      
      if(backChange && !confirmChange){
        nextState = 5; // Back to set minutes
      }
      if(!backChange && confirmChange){
        nextState = 7; // back to set minutes
      }
      break; 

    case 7: // Alarm Schedule Function
      setAlarmSchedule();

      if(backChange && !confirmChange){
        nextState = 6; // Back to AMPM
      }
      if(!backChange && confirmChange){
        nextState = 0; // back to home screen
        Blynk.virtualWrite(V6, alarmHr, alarmMin); // Save values to the blynk Server 
        // TODO: Update ring hours and mins
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
