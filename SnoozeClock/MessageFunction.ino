// Process all functions related to sending, receiving, and processing messages

// Process message from terminal in Blynk App
BLYNK_WRITE(V1){
  
  digitalWrite(inbox, HIGH); // Turn on the indicator lights (unread message)
  led1.on(); 

  // TODO: Send new message notification if on Clock screen (S0)

  table.addRow(tableIndex, param.asStr(), "N/A"); // No response yet
  addMessage(param.asStr()); // Add message to messages array
  Serial.print("Current index: "); 
  Serial.println(tableIndex); 
  tableIndex ++; 
  Blynk.virtualWrite(V5, tableIndex); // Save new index to the server

  // Auto-refresh if on messages page
  if(currentState == 1){
    messagesOverview(); 
  }
}

// push new message to the front of the array
// shift back rest of array, losing oldest message
void addMessage(String newOne) {
  String newMessage = newOne; 
  String oldMessage; 

  // Push new message to top of messages array
  for(int i=0; i < MAX_MESSAGES-1 ; i++){
    oldMessage = messages[i]; 
    messages[i] = newMessage; 
    newMessage = oldMessage;  
  }

  // Store new messages structure to the Blynk server
  Blynk.virtualWrite(V7, messages[0], messages[1], messages[2], messages[3], messages[4], messages[5], messages[6], messages[7], messages[8], messages[9]); 
}

// Message Response
// Notify blynk app, update table value 
void messageRespond(){

  // TODO: send response, log in archive table, send push notification, disable further response (?)
  // Hoooo boy that can't be easy to do
  
}
