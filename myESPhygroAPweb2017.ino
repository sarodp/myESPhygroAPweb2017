//http://esp8266.fancon.cz/esp8266-web-hygrometer-dht11-arduino/esp8266-web-hygrometer-dht11-arduino.html
//
// WiFi.SoftAP Class
// http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html#softap
//
//
// DHT11 hygrometer with GUI ver.2.0.0
// Copyright 2017 Pavel Janko, www.fancon.cz
// MIT license, http://opensource.org/licenses/MIT
//*********************************************************************
// Add libraries DHT sensor library 1.3.0 by Adafruit and
// Adafruit Unified Sensor 1.0.2 from
// Arduino IDE - Sketch/Include Library/Manage Libraries !
//
//*********************************************************************
// Add DS1820 sensor with libraries -- OneWire.h, DallasTemperature.h
//
//*********************************************************************
// DS1820+ESP01 wiring
//
// DS1820                             ESP-01
// ---------                          ------
// pin1.left =GND  -------------------GND 
// pin2.mid  =Data -------------------GPIO2      
// pin3.right=VCC 3.3v-5V-------------3V3 
//
//*********************************************************************
// DHT11/DHT22+ESP01 wiring
//
// DHT11,DHT                     ESP-01
// ---------                     ------
// pin.GND    -------------------GND 
// pin.Data   -------------------GPIO2      
// pin.VCC =3.3v-5V?-------------3V3 
//
//*********************************************************************
// online svg-editor
// *** http://www.drawsvg.org/drawsvg.html
//
//
// svg basic tutorial
// https://www.w3schools.com/html/html5_svg.asp
//
// svg animation sample
// https://www.w3.org/TR/SVG/animate.html#AnimateTransformElement
//
// svg animation tutorial
// http://taye.me/blog/svg/a-guide-to-svg-use-elements/
//
//*********************************************************************

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Ticker.h>              // For periodic events
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

//00---define DHT11 sensor pin
#define DHTTYPE DHT11 // Set sensor type - DHT11 or DHT22
#define DHTPIN 2      // Set pin for sensor connecting
DHT dht(DHTPIN, DHTTYPE);

//00---define DS1820 sensor pin
#define ONE_WIRE_BUS 2  // Data wire is plugged into ESP01.GPIO2
#define TEMPERATURE_PRECISION 9 // Lower resolution
int numberOfDevices; // Number of temperature devices found

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature. 
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

//000---init router constants
const char* ssid =     "MYDLINK";   //my router SSID
const char* password = "20572057";  //my router Password


//01--init var's
bool gradeF = false;   // Set scale: false = Celsius , true = Fahrenheit
String GradeUnit;
String buff;
float  temperature;
float  humidity;
int    FloatRounding  = 1;
float  PointerAngle;
bool   ReadSensorFlag = true;
int    SensorPeriod   = 5;       // Period of sensor reading in seconds
int    PageRefreshPeriod  = 5;  // Period of web page refresh in seconds

//02--init objectvar's
Ticker ReadSensorTick;           // Preparing for periodic sensor reading
ESP8266WebServer MyServer(80);

//===SetUp=====================
void ReadSensorFlagSet(void) {   // Periodic flag setting
  ReadSensorFlag = true;
}

void setup(void) {
//00--init Serial
  Serial.begin(74880);
  Serial.println("");

//01--init TempSensor
  if (gradeF) {
    GradeUnit = "F";             // Setting the temperature scale to fahrenheit
  }
  else {
    GradeUnit = "C";             // or celsius
  }
//01aa--init DS1820
  initDS1820();


//01a---init Ticker
  ReadSensorTick.attach(SensorPeriod, ReadSensorFlagSet); // Periodic flag setting

//02---init WiFi.AP
//*****************************************************
const IPAddress APlocal_IP(192,168,4,22);
const IPAddress APgateway(192,168,4,9);
const IPAddress APsubnet(255,255,255,0);

const char* APssid = "SarodAP";
const char* APpassword = "11112222"; //should be at least 8 character long

const int APchannel = 1;  //Wi-Fi channel, from 1 to 13. Default channel = 1.
const bool APhidden = false; //default = false = show

if (true)
{
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(APlocal_IP, APgateway, APsubnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(APssid,APpassword,APchannel,APhidden) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}
else
{  IPAddress myIP = WiFi.softAPIP();   //use all default softAP.Config()
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}


//02A--init WiFi.Router
if (false) 
{
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
}

//03--init MyWebserver
  initMyWebServer();

//03A--Start Webserver
  MyServer.begin();
  Serial.println("Web server started!");
  Serial.println("-----------------------------");


}  

void initMyWebServer() {
 //...honme
 MyServer.on("/", HTTP_GET, []() {
    buff  = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head>\n";
    buff += "<meta http-equiv=\"refresh\" content=\"" + String(PageRefreshPeriod) + "\">\n";
    buff += "<title>Hygrometer</title></head><body style=\"background:silver\">\n";
    buff += "<h1>Swimming Pool - 1 </h1>";
    buff += "<img src=\"hygro.svg\" alt=\"Hygrometer\">\n";
    buff += "<br></body></html>\n";
    MyServer.send(200, "text/html", buff);
  });

 //.../data
 MyServer.on("/data", HTTP_GET, []() {
    buff  = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head>\n";
    buff += "<meta http-equiv=\"refresh\" content=\"" + String(PageRefreshPeriod) + "\">\n";
    buff += "<title>Hygrometer</title></head><body style=\"background:silver\">\n";
    buff += "<h1>Swimming Pool - 1: Raw Data </h1>\n";
    buff += "<h1> Temp" + GradeUnit + " = " + String(temperature) + "</h1>\n";
    buff += "<h1> Humidity % = " + String(humidity) + "</h1>\n";
    buff += "<br></body></html>\n";
    MyServer.send(200, "text/html", buff);
  });

//.../hygro.svg ===>SVG picture generator
 MyServer.on("/hygro.svg", HTTP_GET, []() {
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
    
    //--temerature Value and GradeUnit
    buff += "<text x=\"465\" y=\"623\" fill=\"brown\">" + String(temperature, FloatRounding) + "</text>\n";
    buff += "<text x=\"560\" y=\"620\" font-size=\"28\">" + GradeUnit + "</text>\n";
    
    //--humidity value
    buff += "<text x=\"465\" y=\"668\" fill=\"blue\">" + String(humidity, FloatRounding) + "</text>\n";
    buff += "<circle cx=\"560\" cy=\"595\" r=\"3\" fill=\"white\" stroke=\"black\" stroke-width=\"2\"/></g>\n";
    buff += "<g stroke=\"red\" stroke-width=\"1\" fill=\"none\">\n";
    
    //--PointerAngle
    buff += "<use xlink:href=\"#PO\" transform=\"rotate(" + String(PointerAngle) + ",500,500)\"/></g></svg>\n";
    MyServer.send(200, "image/svg+xml", buff);
  });

//...NotFound
  MyServer.onNotFound (FileNotFound);          // Fast serves answer 404 File not found as a favicon.ico etc...
}

void FileNotFound() {
  MyServer.send(404, "text/plain", "Page not found !");
}

//===main loop==============
void loop(void) {
  MyServer.handleClient();
  if (ReadSensorFlag == true) {              // Test for periodic reading sensor
    //GetTempDHT();
    GetTempDS1820();
  }
}

//===Get..DHT=========================
void GetTempDHT() {
  temperature = dht.readTemperature(gradeF);  // Read temperature as Celsius or Fahrenheit
  humidity = dht.readHumidity();             // Read humidity (percent)
  PointerAngle = 40 + humidity * 2.8;        // Calculating angle of pointer from humidity
  Serial.print("Temperature " + GradeUnit + " :");
  Serial.println(temperature);
  Serial.print("Humidity    % :");
  Serial.println(humidity);
  Serial.println(" ");
  ReadSensorFlag = false;
}

//===Get..DS1820======================
void initDS1820(void) {
  // Start up the library
  sensors.begin();
  
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  
  // locate devices on the bus
  Serial.print("Locating devices...");
  
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++)
  { // Search the wire for address
   if(sensors.getAddress(tempDeviceAddress, i))
    {
     Serial.print("Found device ");
     Serial.print(i, DEC);
     Serial.print(" with address: ");
     printAddress(tempDeviceAddress);
     Serial.println();
    
     Serial.print("Setting resolution to ");
     Serial.println(TEMPERATURE_PRECISION, DEC);
    
     // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
     sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    
     Serial.print("Resolution actually set to: ");
     Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
     Serial.println();
    }
   else
    {
     Serial.print("Found ghost device at ");
     Serial.print(i, DEC);
     Serial.print(" but could not detect address. Check power and cabling");
    }
  }
  Serial.println("--------------------------------");
}

//===function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

//===print DS1820 temperature
void printTempDS1820(DeviceAddress deviceAddress) {
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress)); // Makes first call to getTempC
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes second call to getTempF

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(" Temp.C = ");
  Serial.print(tempC);
  Serial.print(" Temp.F = ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

//===get DS1820 temperatures
void GetTempDS1820() {
//--issue a global temperature request to all DS1820's on the bus
 Serial.print("Requesting temperatures...");
 sensors.requestTemperatures(); // Send the command to get temperatures
 Serial.println("DONE");

//--print numberOfDevices
 Serial.print("Total Connected Devices = ");
 Serial.println(numberOfDevices, DEC);
  
//--Loop through each device, print out temperature data
 for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
  if(sensors.getAddress(tempDeviceAddress, i)) {
    // Output the device ID
    Serial.print("device #");
    Serial.print(i,DEC);
    
    // It responds almost immediately. Let's print out the data.
    printTempDS1820(tempDeviceAddress); //print debug.data 
    if(gradeF) {
      temperature = sensors.getTempF(tempDeviceAddress);
    } else {
      temperature = sensors.getTempC(tempDeviceAddress);
    }
    
  } 
  //else ghost device! Check your power requirements and cabling
  //print DS1820.error
  }

//--humidity dummy  
 humidity = dht.readHumidity();             // Read humidity (percent)
 //humidity = temperature;        //dummy.debug only
 PointerAngle = 40 + humidity * 2.8;        // Calculating angle of pointer from humidity

//---
 Serial.print("Temperature " + GradeUnit + " :");
 Serial.println(temperature);
 Serial.print("Humidity    % :");
 Serial.println(humidity);
 Serial.println(" ");
 Serial.println("--------------------------------");

//--done
 ReadSensorFlag = false;
}

