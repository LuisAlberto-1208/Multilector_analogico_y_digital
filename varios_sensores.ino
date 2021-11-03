#include <WiFi.h>

const char* ssid     = "INFINITUM4CE0_2.4";
const char* password = "0Qee0zxYbv";

WiFiServer server(80);

String header;

String salidaState = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

const int potPin = 35;
const int fResPin = 34;
const int salida = 19;

const int rgb_R = 23;
const int rgb_G = 22;
const int rgb_B = 21;

int potVal = 0;
int fResVal = 0;

float voltaje(int adc) {
  float v = (3.3/4096)*adc;
  return v;
}
 

void setup() {

  Serial.begin(115200);

  pinMode(salida, OUTPUT);
  pinMode(rgb_R, OUTPUT);
  pinMode(rgb_G, OUTPUT);
  pinMode(rgb_B, OUTPUT);

  digitalWrite(salida, LOW);
  digitalWrite(rgb_R, HIGH);
  digitalWrite(rgb_G, HIGH);
  digitalWrite(rgb_B, HIGH);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Imprime la dirección IP e inicia la Web
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

}

void loop() {

  potVal = analogRead(potPin);
  fResVal = analogRead(fResPin);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
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

            // Enciende y apaga los LED
            if (header.indexOf("GET /salida/on") >= 0) {
              Serial.println("Salida on");
              salidaState = "on";
              digitalWrite(salida, HIGH);
              digitalWrite(rgb_G, LOW);
            } else if (header.indexOf("GET /salida/off") >= 0) {
              Serial.println("Salida off");
              salidaState = "off";
              digitalWrite(salida, LOW);
              digitalWrite(rgb_G, HIGH);
            }

            // Página wrb
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv=\"refresh\" content=\"3\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Cabecera de la página
            client.println("<body><h1>Multilector de datos anal&oacutegicos y uno digital</h1>");

            client.println("<p>Salida - State " + salidaState + "</p>");   
            if (salidaState=="off") {
              client.println("<p><a href=\"/salida/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/salida/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
            client.println("<h2>_______________________________________________</h2>");
            client.println("<h3> Potenci&oacutemetro (ADC) = ");
            client.println(potVal);
            client.println("</h3>");
            client.println("<h3> Voltaje en el potenci&oacutemetro = ");
            client.println(voltaje(potVal));
            client.println(" V</h3>");
            client.println("<h3>Fotorresistencia (ADC) = ");
            client.println(fResVal);
            client.println("</h3>");
            client.println("<h3>Voltaje en la fotorresistencia = ");
            client.println(voltaje(fResVal));
            client.println(" V</h3>");
            client.println("<h2>_______________________________________________</h2>");
            client.println("<h4>Criterios para encender la salida: </h4>");
            client.println("<h5>(1) Por el bot&oacuten en esta p&aacutegina web </h5>");
            client.println("<h5>(2) Cuando el ADC del potenci&oacutemetro est&aacute entre 900 y 1100 </h5>");
            client.println("<h5>(3) Cuando el ADC de la fotorresistencia est&aacute entre 3900 y 4100 </h5>");
            client.println("<h6>Para saber qu&eacute est&iacutemulo encendi&oacute la salida se tiene un LED RGB donde (1) es verde, (2) es rojo y (3) es morado </h6>");
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

  if((potVal <= 1100 and potVal >= 900) or (fResVal <= 4100 and fResVal >= 3900)) {
    digitalWrite(salida, HIGH);

    if(potVal <= 1100 and potVal >= 900) {
      digitalWrite(rgb_R, LOW);
    }
    else if(fResVal <= 4100 and fResVal >= 3900) {
      digitalWrite(rgb_B, LOW);
      digitalWrite(rgb_R, LOW);
    }
    
  }
  else if(salidaState == "off") {
    digitalWrite(salida, LOW);
    digitalWrite(rgb_R, HIGH);
    digitalWrite(rgb_B, HIGH);
    digitalWrite(rgb_G, HIGH);
  }

}
