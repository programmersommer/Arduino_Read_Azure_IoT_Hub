/*
  Forked from
  https://github.com/mohankrr/Arduino/tree/master/MKR1000Azure
  related to article
  https://habrahabr.ru/post/303866/
*/

#include <SPI.h>
#include <WiFi101.h>

const int MKR1000_LED = 6 ;

///*** WiFi Network Config ***///
char ssid[] = "xxx"; //  your network SSID (name)
char pass[] = "xxxxxxxx";    // your network password (use for WPA, or use as key for WEP)


char hostname[] = "xxxxxx.azure-devices.net";    // host name address for your Azure IoT Hub
char feeduri[] = "/devices/xxxxxxx/messages/devicebound?api-version=2016-02-03"; //feed URI
char authSAS[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

///*** Azure IoT Hub Config ***///

unsigned long lastConnectionTime = 0;
const unsigned long pollingInterval = 5L * 1000L; // 5 sec polling delay, in milliseconds

int status = WL_IDLE_STATUS;

WiFiSSLClient client;

void setup() {

  pinMode(MKR1000_LED, OUTPUT);

  //check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
}

void loop()
{
  String response = "";
  char c;
  ///read response if WiFi Client is available
  while (client.available()) {
    c = client.read();
    response.concat(c);
  }

  if (!response.equals(""))
  {
    //if there are no messages in the IoT Hub Device queue, Azure will return 204 status code.
    if (response.startsWith("HTTP/1.1 204"))
    {
      Serial.println("No messages");
      digitalWrite(MKR1000_LED, LOW);
    }
    else
    {
      String message = "";
      if (response.indexOf("Connection: close") > -1)
        message = response.substring(response.indexOf("Connection: close") + 17); // 17 is length of "Connection: close" string
      message.trim();
      
      Serial.println("Message: " + message);
      if (message == "light on")
      {
        digitalWrite(MKR1000_LED, HIGH);
      }
    }
  }

  // polling..if pollingInterval has passed
  if (millis() - lastConnectionTime > pollingInterval)
  {
    Serial.println("Request data from Azure ");
    azureHttpRequest();
  }
}

// this method makes an HTTPS connection to the Azure IOT Hub Server:
void azureHttpRequest() {

  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(hostname, 443)) {
    //make the GET request to the Azure IOT device feed uri
    client.print("GET ");  //Do a GET
    client.print(feeduri);  // On the feedURI
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hostname);  //with hostname header
    client.print("Authorization: ");
    client.println(authSAS);  //Authorization SAS token obtained from Azure IoT device explorer
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
