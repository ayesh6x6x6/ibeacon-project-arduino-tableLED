#include <WiFi.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"

#ifndef BUILTIN_LED
#define BUILTIN_LED  4
#endif

// Update these with values suitable for your network.

const char* ssid = "pollution";
const char* password = "aus12345";
const char* mqtt_server = "192.168.1.128";

// Set up the rgb led names
uint8_t ledR = 22;
uint8_t ledG = 23; // internally pulled up
uint8_t ledB = 15; 

uint8_t flag= 0;

uint8_t ledArray[3] = {1, 2, 3}; // three led channels

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
//  Serial.print(payload);
  String zone = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    zone += (char)payload[i];
    flag = 1;
  }
    StaticJsonBuffer<300> JSONBuffer;                         //Memory pool
    JsonObject& parsed = JSONBuffer.parseObject(zone); //Parse message

    if (!parsed.success()) {   //Check for errors in parsing
 
    Serial.println("Parsing failed");
    delay(5000);
    return;
 
  }
  String actualzone = parsed["zone"];
  int r = parsed["r"];
  int g = parsed["g"];
  int b = parsed["b"];
  if(actualzone == "58350"){
     ledcWrite(1, r); // write red component to channel 1, etc.
    ledcWrite(2, g);   
    ledcWrite(3, b);
  } else if(actualzone == "Finish 58350") {
    ledcWrite(1, 255); // write red component to channel 1, etc.
    ledcWrite(2, 255);   
    ledcWrite(3, 255);
  }
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("cafe/booktable/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
//  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ledcAttachPin(ledR, 1); // assign RGB led pins to channels
  ledcAttachPin(ledG, 2);
  ledcAttachPin(ledB, 3);

    // Initialize channels 
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);
//
//   for(uint8_t i=0; i < 3; i++) {
//  // ledcWrite(channel, dutycycle)
//  // For 8-bit resolution duty cycle is 0 - 255
//  ledcWrite(ledArray[i], 255);  // test high output of all leds in sequence
//  delay(1000);
//  ledcWrite(ledArray[i], 0);
// }
}

void loop() {
//  Serial.println("Initial flag is:" + flag);
  if (!client.connected()) {
    reconnect();
  }
  if(flag == 1){

  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    
    client.publish("outTopic", msg);
  }
}
