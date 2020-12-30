// FSM change based on button presses
void stateChange(){
  
  // Button override to turn off alarm
  if(isRinging){
    
    // back button : snooze for n mins
    if(backChange && !confirmChange){
      isRinging = false; // Turn off alarm until next invoked ring
      digitalWrite(12, LOW); 
      addTime(); // Function to add snooze time until next alarm ringing
    }
    // confirm: turn off alarm
    if(!backChange && confirmChange){
      isRinging = false; 
      digitalWrite(12, LOW); // Stand-in for alarm
      ringMin = alarmMin; // Reset alarm to ring next at user-set time
      ringHr = alarmHr;
    }
    backChange = false; 
    confirmChange = false; 
    return; 
    
  }


  // Continue to conventional screen change
  switch(currentState){ 
    case 0: // Main Clock screen
    
      // TODO: display current time GUI
      
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
      
    case 3: // Set Alarm Screen
  
      // TODO: set alarm GUI settings
//      setAlarm(); 
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock
        Serial.println(nextState); 
        clockDisplay();
      }
      if(!backChange && confirmChange){
        nextState = 0; // Confirm settings + return to clock
        Serial.println(nextState); 
        clockDisplay(); 
      }
      break;
      
    default: 
      // TODO
      // We will default to the clock screen
      break;
  }

  backChange = false; 
  confirmChange = false; 
  currentState = nextState; 
}
