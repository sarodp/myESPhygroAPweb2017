//http://esp8266.fancon.cz/esp8266-web-hygrometer-dht11-arduino/esp8266-web-hygrometer-dht11-arduino.html
//
// DHT11 hygrometer with GUI ver.2.0.0
// Copyright 2017 Pavel Janko, www.fancon.cz
// MIT license, http://opensource.org/licenses/MIT
//*********************************************************************
// Add libraries DHT sensor library 1.3.0 by Adafruit and
// Adafruit Unified Sensor 1.0.2 from
// Arduino IDE - Sketch/Include Library/Manage Libraries !
//*********************************************************************

#include <DHT.h>
#include <Ticker.h>              // For periodic events
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

//000---init constants 
const char* ssid =     "MYDLINK";   //Set your router SSID
const char* password = "20572057";  //Set your router Password

bool grade = false;   // Set scale - false = Celsius , true = Fahrenheit
#define DHTTYPE DHT11 // Set sensor type - DHT11 or DHT22
#define DHTPIN 2      // Set pin for sensor connecting

//01--init var's
String GradeUnit;
String buff;
float  temperature;
float  humidity;
int    FloatRounding  = 1;
float  PointerAngle;
bool   ReadSensorFlag = true;
int    SensorPeriod   = 5;       // Period of sensor reading in seconds
int    PageRefreshPeriod  = 5;  // Period of web page refresh in seconds

//02--init objects
ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

Ticker ReadSensorTick;           // Preparing for periodic sensor reading

void ReadSensorFlagSet(void) {   // Periodic flag setting
  ReadSensorFlag = true;
}
void setup(void) {
//--GradeUnit
  if (grade) {
    GradeUnit = "F";             // Setting the temperature scale to fahrenheit
  } else {
    GradeUnit = "C";             // or celsius
  }
  Serial.begin(74880);
//---WiFi = AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
/*  
//--WiFi = Router
  WiFi.begin(ssid, password);    // Connect to router
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 */

 //--SensorTick
  ReadSensorTick.attach(SensorPeriod, ReadSensorFlagSet); // Periodic flag setting
 
 //1a--http main page
  server.on("/", HTTP_GET, []() {
    buff  = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head>\n";
    buff += "<meta http-equiv=\"refresh\" content=\"" + String(PageRefreshPeriod) + "\">\n";
    buff += "<title>Hygrometer</title></head><body style=\"background:silver\">\n";
    buff += "<h1>Swimming Pool - 1 </h1>";
    buff += "<img src=\"hygro.svg\" alt=\"Hygrometer\">\n";
    buff += "<br></body></html>\n";
    server.send(200, "text/html", buff);
  });

//1b--SVG picture generator
 server.on("/hygro.svg", HTTP_GET, []() {
    buff  = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"1000\" height=\"1000\">\n";
    buff += "<circle cx=\"500\" cy=\"500\" r=\"265\" fill=\"WhiteSmoke\" stroke=\"#b46b41\" stroke-width=\"16\"/>\n";
    buff += "<circle cx=\"500\" cy=\"500\" r=\"265\" fill-opacity=\"0\" stroke=\"#c68762\" stroke-width=\"4\"/>\n";
    buff += "<g stroke=\"black\" stroke-width=\"2\" fill=\"none\">\n";
    buff += "<rect x=\"450\" y=\"590\" rx=\"5\" ry=\"5\" width=\"100\" height=\"40\"/>\n";
    buff += "<rect x=\"450\" y=\"635\" rx=\"5\" ry=\"5\" width=\"100\" height=\"40\"/></g>\n";
    buff += "<path fill=\"none\" stroke=\"#2e8b57\" stroke-width=\"20\" d=\"M413,331 A190,190 35 0 1 657,393\" />\n";
    buff += "<defs>\n";
    buff += "<g id=\"PO\">\n";
    buff += "<line stroke-width=\"5\" x1=\"500\" y1=\"500\" x2=\"500\" y2=\"700\" fill=\"orangered\"/>\n";
    buff += "<rect x=\"492\" y=\"455\" rx=\"2\" ry=\"2\" width=\"16\" height=\"64\" fill=\"orangered\"/>\n";
    buff += "<circle cx=\"500\" cy=\"500\" r=\"2\" stroke=\"brown\" stroke-width=\"4\" />\n";
    buff += "<circle cx=\"500\" cy=\"500\" r=\"3\" fill=\"grey\"/></g>\n";
    buff += "<g>\n";
    buff += "<line stroke-width=\"4\" id=\"M\" x1=\"500\" y1=\"680\" x2=\"500\" y2=\"700\"/>\n";
    buff += "<line stroke-width=\"6\" id=\"L\" x1=\"500\" y1=\"670\" x2=\"500\" y2=\"700\"/></g>\n";
    buff += "<g id=\"Z\">\n";
    buff += "<use xlink:href=\"#L\" transform=\"rotate(0,500,500)\"/>\n";
    buff += "<use xlink:href=\"#M\" transform=\"rotate(14,500,500)\"/></g></defs>\n";
    buff += "<g id=\"T\" stroke=\"black\">\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(40,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(68,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(96,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(124,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(152,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(180,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(208,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(236,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(264,500,500)\"/>\n";
    buff += "<use xlink:href=\"#Z\" transform=\"rotate(292,500,500)\"/>\n";
    buff += "<use xlink:href=\"#L\" transform=\"rotate(320,500,500)\"/></g>\n";
    buff += "<g font-family=\"arial\" font-size=\"36\" font-weight=\"normal\" font-style=\"normal\" stroke=\"black\" fill=\"black\" >\n";
    buff += "<text x=\"350\" y=\"685\">0</text>\n";
    buff += "<text x=\"270\" y=\"590\">10</text>\n";
    buff += "<text x=\"255\" y=\"490\">20</text>\n";
    buff += "<text x=\"290\" y=\"395\">30</text>\n";
    buff += "<text x=\"378\" y=\"315\">40</text>\n";
    buff += "<text x=\"480\" y=\"295\">50</text>\n";
    buff += "<text x=\"577\" y=\"316\">60</text>\n";
    buff += "<text x=\"660\" y=\"380\">70</text>\n";
    buff += "<text x=\"705\" y=\"490\">80</text>\n";
    buff += "<text x=\"690\" y=\"595\">90</text>\n";
    buff += "<text x=\"610\" y=\"690\">100</text>\n";
    buff += "<text x=\"450\" y=\"720\">% RH</text>\n";
    buff += "<text x=\"465\" y=\"623\" fill=\"brown\">" + String(temperature, FloatRounding) + "</text>\n";
    buff += "<text x=\"560\" y=\"620\" font-size=\"28\">" + GradeUnit + "</text>\n";
    buff += "<text x=\"465\" y=\"668\" fill=\"blue\">" + String(humidity, FloatRounding) + "</text>\n";
    buff += "<circle cx=\"560\" cy=\"595\" r=\"3\" fill=\"white\" stroke=\"black\" stroke-width=\"2\"/></g>\n";
    buff += "<g stroke=\"red\" stroke-width=\"1\" fill=\"none\">\n";
    buff += "<use xlink:href=\"#PO\" transform=\"rotate(" + String(PointerAngle) + ",500,500)\"/></g></svg>\n";
    server.send(200, "image/svg+xml", buff);
  });

//1c--NotFound
  server.onNotFound (FileNotFound);          // Fast serves answer 404 File not found as a favicon.ico etc...

//2--Start Webserver
  server.begin();
  Serial.println("Web server started!");
}

void FileNotFound(void) {
  server.send(404, "text/plain", "Page not found !");
}

void loop(void) {
  server.handleClient();
  if (ReadSensorFlag == true) {              // Test for periodic reading sensor
    GetTemperature();
  }
}


void GetTemperature() {
  temperature = dht.readTemperature(grade);  // Read temperature as Celsius or Fahrenheit
  humidity = dht.readHumidity();             // Read humidity (percent)
  PointerAngle = 40 + humidity * 2.8;        // Calculating angle of pointer from humidity
  Serial.print("Temperature " + GradeUnit + " :");
  Serial.println(temperature);
  Serial.print("Humidity    % :");
  Serial.println(humidity);
  Serial.println(" ");
  ReadSensorFlag = false;
}

