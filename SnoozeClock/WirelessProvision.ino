
void wifiProvision() {
  unsigned long timeout = 5000; // One minute
  WiFiMulti wfmulti; 
  Serial.println("Provisioning Wifi"); 
  preferences.begin("conns", false);
  unsigned int numConnections = preferences.getUInt("count", 0);
  
  if(numConnections == 0){
    smartConfig(); // We need to set up a connection
  }

  for(int i=1; i < numConnections+1; i++){
    String net = "conn"; net.concat(i); 
    String pas = "pass"; pas.concat(i);

    #ifdef SERIAL_DEBUGGING
    String APssid = preferences.getString(const_cast<char*>(net.c_str())); 
    String APpsk = preferences.getString(const_cast<char*>(pas.c_str()));
    #endif
    
    Serial.print(i);
    Serial.println(". SSID: " + APssid + ", Pass: " + APpsk);
    wfmulti.addAP(const_cast<char*>(APssid.c_str()), const_cast<char*>(APpsk.c_str())); 
  }
  preferences.end();
  
  if(wfmulti.run(timeout) == WL_CONNECTED) {
      #ifdef SERIAL_DEBUGGING
      Serial.println("\nWiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      #endif
      
      getWifi.ssid = WiFi.SSID();
      getWifi.pass = WiFi.psk();
      WiFi.disconnect(); // Revert to AP mode
  }
  else{
     smartConfig(); // We need to add a new connection
  }
}


void smartConfig(){

  // TODO: Display notice + QR code for smartConfig option
  
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
    preferences.begin("conns", false); // Begin non-volatile storage
    unsigned int num = preferences.getUInt("count", 0); 
    num ++; 
    preferences.putUInt("count", num); // We have a new connection
    
    String newSSID = WiFi.SSID();
    String newPass = WiFi.psk(); ]

    #ifdef SERIAL_DEBUGGING
    Serial.println("New: " + newSSID + ", " + newPass); 
    #endif

    String net = "conn"; net.concat(num); 
    String pas = "pass"; pas.concat(num);  
    preferences.putString(const_cast<char*>(net.c_str()), newSSID); // Store new connection and password in non-volatile
    preferences.putString(const_cast<char*>(pas.c_str()), newPass); 
    
    preferences.end();

    getWifi.ssid = newSSID; 
    getWifi.pass = newPass; 
  }
  
  WiFi.stopSmartConfig();
  WiFi.disconnect();
}
