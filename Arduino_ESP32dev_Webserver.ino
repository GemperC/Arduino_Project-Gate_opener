// Load Wi-Fi library
#include <WiFi.h>
#include <ESP32Servo.h>

// Network credentials
const char* ssid = "";
const char* password = "";
// Set Static IP address
IPAddress local_IP();
// Set Gateway IP address
IPAddress gateway();
// Subnet mask
IPAddress subnet();




// Set web server port number to 80
WiFiServer server();

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String gateState = "CLOSED";

// Assign output variables to IO pin
const int output26 = 26;

// Servo def
Servo motor;
int pos = 0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
  Serial.println("STA Failed to configure");
  }
  Serial.begin(115200);
  // Initialize the output variables as outputs
  //pinMode(output26, OUTPUT);
  pinMode(2, OUTPUT);
  motor.attach(output26); // attach the servo to the IO
 
  // Set outputs to LOW
  //digitalWrite(output26, LOW);
 
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
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP address: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
  
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the IOs on and off
            if (header.indexOf("GET /open") >= 0) {
              gateState = "OPEN";
              //digitalWrite(output26, HIGH);
              //digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
              motor.write(180);
            } else if (header.indexOf("GET /close") >= 0) {
              gateState = "CLOSED";
              //digitalWrite(output26, LOW);
              //digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
              motor.write(0);
            } 
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<body style=background-color:#212121;>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #AF4C50;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1 style=color:white;>Home Gate</h1>");
            
            
            // Display current state, and ON/OFF buttons for IO  
            client.println("<p style=color:white;>The gate is " + gateState + "</p>");
            // If the gate is CLOSED, it displays the OPEN the gate button       
            if (gateState=="CLOSED") {
              client.println("<p><a href=\"/open\"><button class=\"button\">OPEN the gate</button></a></p>");
            } else {
              client.println("<p><a href=\"/close\"><button class=\"button button2\">CLOSE the gate</button></a></p>");
            } 
               
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
