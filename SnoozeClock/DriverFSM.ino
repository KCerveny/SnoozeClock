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

    // ALARM SETTINGS: States 3,4,5,6
    case 3: // Alarm Enable Screen
      // Selection functions
      /* Enable Alarm
       * If scrollUp && !scrollDown && !alarmSet: 
       *  alarmSet = true; // Cursor is over the "on"
       *  update GUI to reflect that "on" is selected
       */

       /* Disable Alarm
        *  If !scrollUp && scrollDown && alarmSet: 
        *   alarmSet = false; // Cursor is over the "off"
        *   update GUI to reflect that "off" is selected
        */
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock
        Serial.println(nextState); 
        clockDisplay();
      }
      if(!backChange && confirmChange){
        nextState = 4; // Enabled/Disabled, proceed to hour
        Serial.println(nextState); 
        setHoursScreen(); 
      }
      break;

    case 4: // Hour Set Function
      // Hour selection functions
      /* Increment Hour
       * If scrollUp && !scrollDown: 
       *  alarmHr += (alarmHr + 1)%24 // Overflow back to midnight
       *  update GUI to show new hour
       */

       /* Decrement Hour
        *  If !scrollUp && scrollDown: 
        *   alarmHr --; 
        *   if(alarmHr < 0) alarmHr = 23; // roll around back to 11 pm
        *   update GUI to show new hour
        */
      if(backChange && !confirmChange){
        nextState = 3; // Return to Enable
        setAlarmScreen();        
      }
      if(!backChange && confirmChange){
        nextState = 5; // Set minutes next
        setMinutesScreen(); 
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
        setHoursScreen(); 
      }
      if(!backChange && confirmChange){
        nextState = 6; // Set schedule next
        setScheduleScreen(); 
      }
      break; 

    case 6: // Alarm Schedule Function
      // Schedule Select Functions

      // TODO: how to select schedules
      
      if(backChange && !confirmChange){
        nextState = 5; // Back to clock screen
        setMinutesScreen(); 
      }
      if(!backChange && confirmChange){
        nextState = 0; // back to set minutes
        clockDisplay(); 
      }
      break; 
    
    default: 
      // TODO
      // We will default to the clock screen
      break;
  }

  // TODO: scrollUp, scrollDown = false, have been handled in state
  backChange = false; 
  confirmChange = false; 
  
  currentState = nextState; 
}
