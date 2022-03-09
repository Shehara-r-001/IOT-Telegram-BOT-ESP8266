//#ifdef ESP32
//  #include <WiFi.h>
//#else
//  #include <ESP8266WiFi.h>
//#endif
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WifiLocation.h>


// Replace with your network credentials
const char* ssid = "SLT-4G_B53FA";
const char* password = "2251151aasr";

const char* googleApiKey = "AIza5yDBPopc4WAFxkfi3QMIruDAgoCshAvDxA"; //"AIzaSyBDBc_FBEzE5hxDII0uArF7W7p3e71-aVU";
WifiLocation location (googleApiKey);

// Initialize Telegram BOT
#define BOTtoken "5005872462:AAExixA8saSldif-hMOEaXAXTOsU8i4KCN0"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "779385493"            //group id - "-686857845"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = D8;
bool ledState = LOW;

#define DHTPIN D2    // digital pin connected to DHT
#define DHTTYPE    DHT22


DHT dht(DHTPIN, DHTTYPE);
DHT_Unified dht_u(DHTPIN, DHTTYPE);

String image_url = "https://firebasestorage.googleapis.com/v0/b/twitter-clone-52dc6.appspot.com/o/iot-temp%2Fcircuit-sketch-iot.PNG?alt=media&token=9b5ce2fd-62c3-4cd9-aa12-b380507e79d2";

String getReadings(){
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  String message = "Current readings from the sensors : \n";
  message += "Temperature : " + String(temperature) + " C \n";
  message += "Humidity : " + String(humidity) + " % \n";
  Serial.println(message);
  return message;
  
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Please use the following commands to control your outputs.\n\n";
      welcome += "/light01_on to turn Light 01 ON \n";
      welcome += "/light01_off to turn Light 01 OFF \n";
      welcome += "/readings to receive the data from the sensors \n";
      welcome += "/state to request current state of the Light \n";
      welcome += "/options to get the virtual keyboard \n";
      welcome += "/get_sketch to get the sketch of the circuit \n";
      welcome += "/info to get information about the sensors \n";
      welcome += "/location to get the coordinates of the location \n";
      welcome += "/help to get the usage commands \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/help") {
      String instructions = "Hello, " + from_name + ".\n";
      instructions += "Please use the following commands to control your outputs.\n\n";
      instructions += "/light01_on to turn Light ON \n";
      instructions += "/light01_off to turn Light OFF \n";
      instructions += "/readings to receive the data from the sensors \n";
      instructions += "/state to request current state of the Light \n";
      instructions += "/options to get the virtual keyboard \n";
      instructions += "/get_sketch to get the sketch of the circuit \n";
      instructions += "/info to get information about sensors \n";
      instructions += "/location to get the coordinates of the location \n";
      instructions += "/help to get the usage commands \n";
      bot.sendMessage(chat_id, instructions, "");
    }

    if (text == "/light01_on") {
      bot.sendMessage(chat_id, "Light 01 turned ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/light01_off") {
      bot.sendMessage(chat_id, "Light 01 turned OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }

    if (text == "/readings") {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    } 
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "Light 01 is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "Light 01 is OFF", "");
      }
    }

    if (text == "/options"){
      String keyboardJson = "[[\"/light01_on\", \"/light01_off\"],[\"/readings\", \"/state\"], [\"/info\", \"/help\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);
    }

    if (text == "/get_sketch") {
      bot.sendPhoto(chat_id, image_url, "Sketch of the circuit (as of 4 March 22)");
    }

    if (text == "/info") {
      String infoo = tempSensorInfo() + "\n" + humiditySensorInfo();
      bot.sendMessage(chat_id, infoo, "");
    } 

    if (text == "/location"){
      String loc_data = location_data();
      bot.sendMessage(chat_id, loc_data, "");
    }

  }
}

String location_data(){
    location_t loc = location.getGeoFromWiFi();

//      String loc_data = "Location requesting data..";
//      loc_data += location.getSurroundingWiFiJson() + "\n";
      String loc_data = "Latitudes : " + String(loc.lat, 7) + "\n";
      loc_data += "Longitudes : " + String(loc.lon, 7) + "\n";
      loc_data += "Accuracy (in m) : " + String(loc.accuracy) + "\n";
      loc_data += "Result : " + location.wlStatusStr (location.getStatus ());
      Serial.println(loc_data);
      return loc_data;
  }

void bot_setup(){
  const String commands = F("["
                            "{\"command\":\"start\", \"description\":\"Start the chat\"},"
                            "{\"command\":\"status\",\"description\":\"Get the sensor readings\"}," 
                            "{\"command\":\"light01_on\",\"description\":\"Turn on the light 01\"},"
                            "{\"command\":\"light01_off\",\"description\":\"Turn off the light 01\"},"
                            "{\"command\":\"options\",\"description\":\"Open the virtual keyboard\"},"
                            "{\"command\":\"get_sketch\",\"description\":\"Get the sketch of the circuit\"},"
                            "{\"command\":\"info\",\"description\":\"Get the information about the sensors\"},"
                            "{\"command\":\"location\",\"description\":\"Get the coordinates of the location\"},"
                            "{\"command\":\"help\",  \"description\":\"Get bot usage help\"}"  // no comma in the final command
                            "]");
  bot.setMyCommands(commands);
  
}

String tempSensorInfo(){
  
  sensor_t sensor;
  
  dht_u.temperature().getSensor(&sensor);
  String information = "Temperature Sensor \n";
  information += "- Sensor Type : " + String(sensor.name) + " \n";
  information += "- Driver Ver : " + String(sensor.version) + " \n";
  information += "- Unique ID: " + String(sensor.sensor_id) + " \n";
  information += "- Max Value : " + String(sensor.max_value) + "°C" + " \n";
  information += "- Min Value : " + String(sensor.min_value) + "°C" + " \n";
  information += "- Resolution : " + String(sensor.resolution) + "°C" + " \n";

  Serial.println(information);
  return information;
  }

String humiditySensorInfo(){

  sensor_t sensor;

  dht_u.humidity().getSensor(&sensor);
  String information = "Humidity Sensor \n";
  information += "- Sensor Type : " + String(sensor.name) + " \n";
  information += "- Driver Ver : " + String(sensor.version) + " \n";
  information += "- Unique ID: " + String(sensor.sensor_id) + " \n";
  information += "- Max Value : " + String(sensor.max_value) + "%" + " \n";
  information += "- Min Value : " + String(sensor.min_value) + "%" + " \n";
  information += "- Resolution : " + String(sensor.resolution) + "%" + " \n";

  Serial.println(information);
  return information;
  
  }


void setup() {
  Serial.begin(115200);
  dht.begin();

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  bot_setup();

}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
