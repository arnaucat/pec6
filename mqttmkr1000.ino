#include <Arduino_MKRENV.h>
//#include "arduino_secrets.h" xplicit..
#include <PubSubClient.h>
#include <Homie.h>
//#include <ESP8266WiFi.h>
//#include <AsyncMqttClient.h>


// Update these with values suitable for your network.

const char* ssid = "iPhone";
const char* password = "holapatata";
const char* mqtt_server = "192.168.1.130"; 

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


void regdevnod()
{
  
  //aqui tendriamos que llamar para crear instancia al objeto y crearimos el device y despues los nodos con la class homie
  mqttobj.regdevice(DEVICE_ID, "mkr", "pressure,termosthat,humidity,uv-sensor");
  mqttobj.regnode("homie/mkr/", "pressure", "Pressure sensor", "pressure");
  mqttobj.regnode("homie/mkr/", "termosthat", "Temperature sensor", "temperature");
  mqttobj.regnode("homie/mkr/", "humidity", "Humidity sensor", "humidity");
  mqttobj.regnode("homie/mkr/", "uv-sensor", "UV sensor", "uv-index,ultraviolet-a,ultraviolet-b");
}


void readmkrenv()
{
  float temperature = ENV.readTemperature();
  float humidity = ENV.readHumidity();
  float pressure = ENV.readPressure();
  float uva = ENV.readUVA();
  float uvb = ENV.readUVB();
  float uvIndex = ENV.readUVIndex();
  mqttobj.sendPub("homie/mkr/thermostat/", "temperature", "float", "°C", String(temperature));
  mqttobj.sendPub("homie/mkr/humidity/", "humidity", "float", "%", String(humidity));
  mqttobj.sendPub("homie/mkr/pressure/", "pressure", "float", "kPa", String(pressure));
  mqttobj.sendPub("homie/mkr/uv-sensor/", "ultraviolet-a", "float", "μW/cm2", String(uva));
  mqttobj.sendPub("homie/mkr/uv-sensor/", "ultraviolet-b", "float", "μW/cm2", String(uvb));


}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
      

      
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
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  regdevnod();
  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
   // snprintf (msg, 50, "hello world #%ld", value);
    Serial.print("Publish message: ");
    readmkrenv()
    //Serial.println(msg);
    client.publish("outTopic", msg);
     mqttobj.sendLastWillMessage(DEVICE_ID);
  }

}
