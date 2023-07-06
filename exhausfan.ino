#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <SwitchManager.h>

const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastMsg1 = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
#define MSG_BUFFER_SIZE1  (50)
char msg1[MSG_BUFFER_SIZE1];
int value = 0;
#define ssr1 15  //14
#define ssr2 12
#define ssr3 2
#define ssr4 4
#define sub "iterahero/actuator/8"

int interval1 = 0;
int interval2 = 10000;
int interval3 = 20000;
int interval4 = 30000;
WiFiManager wm;

const byte testSwitch = 16;

SwitchManager mySwitch;

void setup_wifi() {

  delay(10);

  WiFi.mode(WIFI_STA);
      //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("Exhaus_fan","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }


  randomSeed(micros());


  Serial.println(WiFi.localIP());
}
int state;
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if (strstr(topic, sub))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      state = 0;
      digitalWrite(ssr1, LOW);
      digitalWrite(ssr2, LOW);
      delay(10000);
      digitalWrite(ssr3, LOW);
      delay(10000);
      digitalWrite(ssr4, LOW);
      } 
    else if ((char)payload[0] == '1') {
      state = 1;
      digitalWrite(ssr1, HIGH);
      digitalWrite(ssr2, HIGH);
      delay(10000);
      digitalWrite(ssr3, HIGH);
      delay(10000);
      digitalWrite(ssr4, HIGH);
    }    
  }
}

byte willQoS = 1;
const char* willTopic = "iterahero/status/actuator/8";
const char* willMsg = "[{\"id_actuator\":8,\"status\":\"offline\"}]";
boolean willRetain = true;
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
     if (client.connect(clientId.c_str(), willTopic, willQoS, willRetain, willMsg)){
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(willTopic, "[{\"id_actuator\":8,\"status\":\"online\"}]");
      // ... and resubscribe
      client.subscribe(sub);
    } 
  }
}

void setup() {
    pinMode(ssr1, OUTPUT); 
    pinMode(ssr2, OUTPUT);
    pinMode(ssr3, OUTPUT);
    pinMode(ssr4, OUTPUT);
    // put your setup code here, to run once:
    Serial.begin(115200);

    mySwitch.begin (testSwitch, handleSwitchPress);
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

int count;
void handleSwitchPress (const byte newState, const unsigned long interval, const byte whichPin)
{ 
  if (newState == LOW)
  {
    return;
  }

  // switch must be HIGH
  if (interval >= 1000)
  {
    count++;
    if (count == 1){
      state = 1;
      digitalWrite(ssr1, HIGH);
      digitalWrite(ssr2, HIGH);
      delay(10000);
      digitalWrite(ssr3, HIGH);
      delay(10000);
      digitalWrite(ssr4, HIGH);
     }
     else if (count == 2){
      state = 0;
      digitalWrite(ssr1, LOW);
      digitalWrite(ssr2, LOW);
      delay(10000);
      digitalWrite(ssr3, LOW);
      delay(10000);
      digitalWrite(ssr4, LOW);
      count = 0;
     }
  }
}  // end of handleSwitchPress

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
       setup_wifi();
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 3000) {
  lastMsg = now;
  ++value;
  snprintf (msg, MSG_BUFFER_SIZE, "[{\"id_actuator\":8,\"status\":\"online\"}]");
  client.publish(willTopic, msg);
  }
  unsigned long now2 = millis();
  if (now2 - lastMsg1 > 4000) {
    lastMsg1 = now2;
    if (state == 1){
      snprintf (msg1, MSG_BUFFER_SIZE1, "1");
      client.publish("iterahero/respon/actuator/8", msg1);
    }
   else if (state == 0){
      snprintf (msg1, MSG_BUFFER_SIZE1, "0");
      client.publish("iterahero/respon/actuator/8", msg1);
    }
  }

}
