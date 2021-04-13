
void wifiProvision() {
  unsigned long timeout = 5000; // One minute
  WiFiMulti wfmulti; 
  Serial.println("Provisioning Wifi"); 
  preferences.begin("connections", false);
  unsigned int numConnections = preferences.getUInt("count", 0);
  
  if(numConnections == 0){
    smartConfig(); // We need to set up a connection
  }

  for(int i=1; i < numConnections+1; i++){
    String net = "conn"; net.concat(i); 
    String pas = "pass"; pas.concat(i);
    Serial.println("Retreiving: " + net + " " + pas);
    String APssid = preferences.getString(const_cast<char*>(net.c_str())); 
    String APpsk = preferences.getString(const_cast<char*>(pas.c_str()));
    Serial.print(i);
    Serial.println(". SSID: " + APssid + ", Pass: " + APpsk);
    wfmulti.addAP(const_cast<char*>(APssid.c_str()), const_cast<char*>(APssid.c_str())); 
  }
  preferences.end();
  
  if(wfmulti.run(timeout) == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      getWifi.ssid = WiFi.SSID();
      getWifi.pass = WiFi.psk();
  }
  else{
     smartConfig(); // We need to add a new connection
  }
}


void smartConfig(){
    //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  // Define config mode?
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }
  
  #ifdef SERIAL_DEBUGGING
  Serial.println("\nSmartConfig received.");
  #endif

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  #ifdef SERIAL_DEBUGGING
  Serial.println("\nWiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.SSID()); 
  Serial.println(WiFi.psk());
  #endif

  if(WiFi.status() == WL_CONNECTED){
    preferences.begin("connections", false); // Begin non-volatile storage
    unsigned int num = preferences.getUInt("count", 0); 
    num ++; 
    preferences.putUInt("count", num); // We have a new connection
    
    String newSSID = WiFi.SSID();
    String newPass = WiFi.psk(); 
    Serial.println("New: " + newSSID + ", " + newPass); 

    String net = "conn"; net.concat(num); 
    String pas = "pass"; pas.concat(num); 
    Serial.print("Spaces: " + net + ", " + pas); 
    preferences.putString(const_cast<char*>(net.c_str()), newSSID); // Store new connection and password in non-volatile
    preferences.putString(const_cast<char*>(pas.c_str()), newPass); 
    
    preferences.end();

    getWifi.ssid = newSSID; 
    getWifi.pass = newPass; 
  }
  
  WiFi.stopSmartConfig();
  WiFi.disconnect();
}
