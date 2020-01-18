#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* nodeid       = "Miguel";
const char* ssid         = "31EA";
const char* password     = "ko@123456";
const char* mqtt_server  = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void subscribe_topics(){
  client.subscribe("IMH/");
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Esta funcion se ejecuta cada vez que recibimos un mensaje del mqtt
  String t(topic);
  String msg;
  for(int i = 0 ; i<length ; i++)
    msg+=(char)payload[i];
  debugln("Message arrived, Topic: " + t + " msg: " + msg);
}

void debug(const String msg)
{
  Serial.print(msg);
  String topic = "node/" + String(nodeid) + "/debug";
  client.publish(topic.c_str(),msg.c_str());
  client.loop();
  yield();
}

void debugln(const String msg)
{
  debug(msg+"\n");
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(nodeid);
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

String localIP()
{
  return String(WiFi.localIP()[0]) + "." +String(WiFi.localIP()[1]) + "." +String(WiFi.localIP()[2]) + "." +String(WiFi.localIP()[3]);
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(nodeid)) 
    {
      debugln("connected");
      subscribe_topics();
      String pub = String("Hola Mundo! Soy ") + String(nodeid) + ", mi IP es " + localIP();
      client.publish("node/hello",pub.c_str());
    } else {
      debug("failed, rc=");
      debug(String(client.state()));
      debugln(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);

  
  // INICIO WIFI
  setup_wifi();  // descomentar para hardcode
  MDNS.begin(nodeid);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // DHT11
  Serial.println(F("DHTxx test!"));
  dht.begin();
}

void loop() {
  
  // LOOP WIFI
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
 
  String msg_h = String(h);
  String msg_t = String(t);
  client.publish("IMH/h", msg_h.c_str());
  client.publish("IMH/t", msg_t.c_str());
  delay(2000);
}
