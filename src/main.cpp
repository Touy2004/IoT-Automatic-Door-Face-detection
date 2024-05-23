#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>

Servo klabServo;
Servo iotServo;

const char *ssid = "CEIT-IoT-Lab";
const char *password = "1oTCEIT@2022";
const char *mqtt_server = "broker.hivemq.com";
const char *mqtt_topic = "face-detection";

int functionMode = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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

void callback(char *topic, byte *payload, unsigned int length) {
  if ((char)payload[0] == '2') {
    Serial.println("on");
    functionMode = 1;
  } else {
  client.publish(mqtt_topic, "1");
  }
}

void reconnect() {
  while (!client.connected()) {
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  klabServo.setPeriodHertz(50); // standard 50 hz servo
  iotServo.setPeriodHertz(30);
  klabServo.attach(18, 1000, 2100);
  iotServo.attach(19, 1000, 2100);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.println("sended msg");
  delay(1000);
  
  if (functionMode == 1) {
    klabServo.write(180);
    iotServo.write(0);
    delay(5000);
    klabServo.write(0);
    iotServo.write(180);
    functionMode = 0;
  
  } else {
    klabServo.write(0);
    iotServo.write(180);
  }
}
