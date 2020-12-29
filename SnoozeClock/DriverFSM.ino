// FSM change based on button presses
void stateChange(){
  
  switch(currentState){ 
    case 0: // Main Clock screen
    
      // TODO: display current time GUI
      
      if(backChange && !confirmChange){
        nextState = 3; // Set alarm
        Serial.println(nextState); 
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
      }
      break; 
      
    case 2: // Open Message Screen

      // TODO: Open Message GUI
      
      if(backChange && !confirmChange){
        nextState = 1; // Check messages
        Serial.println(nextState); 
        clockDisplay();
      }
      if(!backChange && confirmChange){
        nextState = 1; // Send response + Check messages
        Serial.println(nextState); 
        clockDisplay();
      }
      break; 
      
    case 3: // Set Alarm Screen
  
      // TODO: set alarm GUI settings
      
      if(backChange && !confirmChange){
        nextState = 0; // Clock
        Serial.println(nextState); 
      }
      if(!backChange && confirmChange){
        nextState = 0; // Confirm settings + clock
        Serial.println(nextState); 
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
