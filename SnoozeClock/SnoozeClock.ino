
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"

// UTP Variables
const char* ntpServer = "pool.ntp.org";
const long  cstOffset_sec = -21600;
const int   daylightOffset_sec = 3600;

// Connectivity Credentials
char auth[] = "HVMVCQ6T1ie1ER0uix-iNEZelEf7N82z"; // You should get Auth Token in the Blynk App.
char ssid[] = "ATTcIIbe6a"; // Your WiFi credentials.
char pass[] = "ss7aaffspp#m"; // Set password to "" for open networks.

// Virtual Connections/Pins
WidgetTerminal terminal(V1); // Attach virtual serial terminal to Virtual Pin V1
WidgetLED led1(V2); // Notification LED in Blynk app

// Accesses and prints CST time from UTP server
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M");
}


// Received Message from terminal in Blynk App
BLYNK_WRITE(V1)
{
  digitalWrite(23, HIGH); // Turn on the indicator light
  led1.on(); // Write to blynk app
  
  Serial.print("Incoming: "); // Info sent to the ESP32 from terminal?
  Serial.println(param.asStr());
  // if you type "Marco" into Terminal Widget - it will respond: "Polo:" 
  if (String("Marco") == param.asStr()) {

    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } else {

    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }

  // Ensure everything is sent
  terminal.flush();
}


void setup()
{
  pinMode(23, OUTPUT); // Notification light
  
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);

  terminal.clear();

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-------------"));
  terminal.println(F("Type 'Marco' and get a reply, or type"));
  terminal.println(F("anything else and get it printed back."));
  terminal.flush();
}

void loop()
{
  Blynk.run();
}
