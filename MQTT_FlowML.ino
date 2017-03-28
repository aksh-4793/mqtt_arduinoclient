#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <EEPROM.h>
// Update these with values suitable for your network.

const char* ssid = "Hidden";
const char* password = "R@p+0r@cybernet";
const char* mqtt_server = "192.168.1.102";


//H/W setup
const int buttonPin = D2;
int addr = 0;
float calibrationFactor = 4.5;
volatile byte pulseCount;
float flowRate;
char flow_ml[10];
unsigned int flowMilliLitres;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void pulseCounter(){
  pulseCount++;
}

void setup() {
  pulseCount = 0;
  flowRate   = 0.0;
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 11883);
  // client.setCallback(callback);
  attachInterrupt(digitalPinToInterrupt(buttonPin), pulseCounter, RISING);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe     
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {    
    ++value;
    detachInterrupt(digitalPinToInterrupt(buttonPin));
    flowRate = ((1000.0 / (millis() - lastMsg)) * pulseCount) / calibrationFactor;
    flowMilliLitres = (flowRate / 60) * 1000;
    lastMsg = now;
    pulseCount = 0;
    dtostrf(flowMilliLitres, 4, 2, flow_ml);
    snprintf (msg, 75, "%s",flow_ml);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
    attachInterrupt(digitalPinToInterrupt(buttonPin), pulseCounter, FALLING);
  }
}
