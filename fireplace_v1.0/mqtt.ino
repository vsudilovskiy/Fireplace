//------------------------------------------------------------------------------------------------------------------------------------------------------
void mqttcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


}
//------------------------------------------------------------------------------------------------------------------------------------------------------
void mqttreconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttclient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttclient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttclient.subscribe("home/fireplace/state"); //ON OFF
      mqttclient.subscribe("home/fireplace/ledmode"); //1 2 3 4 5
      mqttclient.subscribe("home/fireplace/ledbrigh"); //ON OFF
      mqttclient.subscribe("home/fireplace/soundstate"); //ON OFF
      mqttclient.subscribe("home/fireplace/soundvol"); //0 - 100 / step 20
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------
