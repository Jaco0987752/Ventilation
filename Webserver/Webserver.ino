// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "credentials.h"
// Set web server port number to 80
ESP8266WebServer server(80);

// Variable to store the HTTP request
String header;


String fromSerial = "{\"empty\":\"empty\"}";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/body", handleBody); //Associate the handler function to the path
  
  server.begin();
}

void handleBody() { //Handler for the body path
 
      if (server.hasArg("plain")== false){ //Check if body received
 
            server.send(200, "text/plain", "Body not received");
            return;
 
      }
 
      String message = server.arg("plain");
             message += "\n";
 
      server.send(200, "text/json", fromSerial);
      Serial.println(message);
}

void loop(){
  server.handleClient(); //Handling of incoming requests

    String temp = Serial.readString();
    
    if(temp.length() > 1){
      fromSerial = temp;
    }
}
