String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("GET error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void getCoords(){
  WiFi.mode(WIFI_MODE_STA); // Set up device as WiFi station to find other devices
  WiFi.begin(ssid, pass);
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
      // wait 5 seconds for connection:
      Serial.print("Status = ");
      Serial.println(WiFi.status());
      delay(600);
  }
  String body = "{\"wifiAccessPoints\":" + getSurroundingWiFiJson() + "}";

  #ifdef SERIAL_DEBUGGING
  Serial.println(body);
  #endif
  
  // Send POST request to Google Service
  HTTPClient goog;
  String googlePath = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + googleApiKey;
  
  goog.begin(googlePath);
  goog.addHeader("Content-Type","application/json", false, false);
  int httpResp = goog.POST(body);
  String payload = "Empty";
  
  if (httpResp > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResp);
    payload = goog.getString();
  }
  else {
    Serial.print("POST Error code: ");
    Serial.println(httpResp);
  }
  goog.end();

  Serial.print(payload);

  JSONVar location = JSON.parse(payload);
  if(JSON.typeof(location) == "undefined"){
    clockLocation.lon = "-97.7392"; // Default to Austin, TX if not found
    clockLocation.lat = "30.2856";
    clockLocation.foundLoc = false;
  }
  else{
    clockLocation.lon = String((double(location["location"]["lng"])), 4); 
    clockLocation.lat = String((double(location["location"]["lat"])), 4); 
    clockLocation.acc = String(int(location["accuracy"]));
    clockLocation.foundLoc = true;
  }
  
  WiFi.disconnect(); // Change WiFi mode for Blynk
  delay(400);
}

String MACtoString(uint8_t* macAddress) {
    uint8_t mac[6];
    char macStr[18] = { 0 };
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    return  String(macStr);
}

// Function to get a list of surrounding WiFi signals in JSON format to get location via Google Location API
String getSurroundingWiFiJson() {
    #ifndef MAX_WIFI_SCAN
    #define MAX_WIFI_SCAN 126
    #endif
    
    String wifiArray = "[\n";

    int8_t numWifi = WiFi.scanNetworks();
    if(numWifi > MAX_WIFI_SCAN) {
        numWifi = MAX_WIFI_SCAN;
    }
    for (uint8_t i = 0; i < numWifi; i++) {
        wifiArray += "{\"macAddress\":\"" + MACtoString(WiFi.BSSID(i)) + "\",";
        wifiArray += "\"signalStrength\":" + String(WiFi.RSSI(i)) + ",";
        wifiArray += "\"channel\":" + String(WiFi.channel(i)) + "}";
        if (i < (numWifi - 1)) {
            wifiArray += ",\n";
        }
    }
    WiFi.scanDelete();
    wifiArray += "]";
    return wifiArray;
}

// Calls openWeather API
// Returns: City, timezone, current weather data
void locationStatus() {
  String apiPath = "https://api.openweathermap.org/data/2.5/weather?lat=" + clockLocation.lat + "&lon=" + clockLocation.lon + "&appid=" + openWeatherMapApiKey;

  #ifdef SERIAL_DEBUGGING
  Serial.println(apiPath);
  #endif
  
  String jsonBuff = httpGETRequest(apiPath.c_str());
  JSONVar newObject = JSON.parse(jsonBuff);
  if (JSON.typeof(newObject) == "undefined") {
    Serial.println("Failed to find current weather info!");
    return;
  }
  String tempCity = JSON.stringify(newObject["name"]);
  String tempCountry = JSON.stringify(newObject["sys"]["country"]);
  tempCity.replace("\"", "");
  tempCountry.replace("\"", ""); 
  clockLocation.city = tempCity;
  clockLocation.country = tempCountry; 
  clockLocation.tzSec = long(newObject["timezone"]);

  #ifdef SERIAL_DEBUGGING
  Serial.println(clockLocation.city + ", " + clockLocation.country); 
  Serial.print("Timezone: ");
  Serial.println(clockLocation.tzSec); 
  #endif
}

// ISR: OpenWeather HTTP request
// Sends HTTP request, parses JSON, stores relevant weather info
void getWeather(){
  if(WiFi.status() != WL_CONNECTED){
    temp = "--";
    precip = "--";
    return;
  }
  
  String serverPath = "https://api.openweathermap.org/data/2.5/onecall?lat=" +clockLocation.lat+ "&lon=" +clockLocation.lon+ "&units=imperial&appid=" +openWeatherMapApiKey;
  jsonBuffer = httpGETRequest(serverPath.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);
  
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    temp = "--";
    precip = "--";
    return;
  }
  double tempRead = round(double(myObject["current"]["temp"]));
  temp = String(int(tempRead)); // Round, truncate decimal, convert to string
  double precipRead = round(double(myObject["hourly"][0]["pop"])*100); 
  precip = String(int(precipRead));
  iconID = (JSON.stringify(myObject["hourly"][0]["weather"][0]["icon"])).substring(1,3); // Get ID of weather icon
  Serial.println("ICON: " + iconID);
  sunrise = long(myObject["current"]["sunrise"]);
  sunset = long(myObject["current"]["sunset"]);

  
  // National weather alerts and warnings
  if(myObject.hasOwnProperty("alert") && !seenAlert){
    seenAlert = true;
    Serial.println("National Weather Alert!");
    String alert = JSON.stringify(myObject["alert"]["sender_name"]) +": "+ JSON.stringify(myObject["alert"]["event"]);
    addMessage(alert); // Weather alert will show up in inbox
  }
  else if(!myObject.hasOwnProperty("alert") && seenAlert){
    seenAlert = false; // We can set flag to false now that the alert is over
  }
}
