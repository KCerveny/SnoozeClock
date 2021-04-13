
void wifiProvision() {
  unsigned long timeout = 60*1000; // One minute
  WiFiMulti wfmulti; 
  preferences.begin("connections", false);
  unsigned int numConnections = preferences.getUInt("count", 0);
  
  if(numConnections == 0){
    smartConfig(); // We need to set up a connection
  }

  for(unsigned int i=0; i < numConnections; i++){
    String net = "conn" + i; 
    String pas = "pass" + i; 
    String APssid = preferences.getString(const_cast<char*>(net.c_str())); 
    String APpsk = preferences.getString(const_cast<char*>(temp.c_str()));
    
    wfmulti.addAP(const_cast<char*>(APssid.c_str()), const_cast<char*>(APssid.c_str())); 
  }

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

    String net = "conn" + num; 
    String pas = "pass" + num; 
    preferences.putString(const_cast<char*>(net.c_str()), newSSID); // Store new connection and password in non-volatile
    preferences.putString(const_cast<char*>(pas.c_str()), newPass); 
    
    preferences.end();

    getWifi.ssid = newSSID; 
    getWifi.pass = newPass; 
  }
  
  WiFi.stopSmartConfig();
  WiFi.disconnect();
}
