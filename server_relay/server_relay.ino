#include <ESP8266WiFi.h>

// 继电器 以及一个光照传感器 模块代码 选择 D1 R2

#define RELAY_PIN_1 D3 //  relay control pin D3

#define RELAY_PIN_2 D5 //  relay control pin D5

String arduinoID = "01";

String RELAY_ID[] = {"00", "01"};

#define OPEN LOW // if relay is low-level trigger

#define CLOSE HIGH

char RELAY_STATE[] = {0, 0}; // save the relay status;

#define PORT 9000

const char* ssid = "alonebo";
const char* password = "976447044";


int lightValue = 0;

int count = 0;

WiFiServer server(PORT);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Pin init...!");

  pinMode(BUILTIN_LED, OUTPUT);

  pinMode(RELAY_PIN_1, OUTPUT);

  pinMode(RELAY_PIN_2, OUTPUT);

  digitalWrite(RELAY_PIN_1, CLOSE);

  digitalWrite(RELAY_PIN_2, CLOSE);

  Serial.printf("Connecting to wifi: %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");

  server.begin();
  Serial.printf("Tcperver started, connect %s\n", WiFi.localIP().toString().c_str());

}

void blinkLed()
{
  digitalWrite(BUILTIN_LED, HIGH);
  delay(300);
  digitalWrite(BUILTIN_LED, LOW);
  delay(300);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(300);
  digitalWrite(BUILTIN_LED, LOW);

}

void blinkLedFast()
{
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);

}

void sendData(WiFiClient client)
{
  lightValue = analogRead(A0);
  String tmp = String(lightValue, DEC);
  String lightData = String(arduinoID + "_40_10_" + tmp);
  client.print(lightData);
  blinkLedFast();
  Serial.println("LightValue:" + tmp);
}

int checkClient(WiFiClient client)
{
  if (client)
  {
    if (client.connected())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        handleData(line, client);
      }
      if (count >= 25)
      {
        sendData(client);
        count = 0;
      }

    } else {
      delay(1);
      client.stop();
      blinkLed();
      Serial.println("[Client disonnected]");
      return -1;
    }
  }
  return 0;
}

void sendRelayStatus(WiFiClient client, int i)
{
    String tmp = String(RELAY_STATE[i], DEC);
    String data = String(arduinoID + "_" + RELAY_ID[i] + "_10_" + tmp);
    client.print(data);
    Serial.println(data);
}

void handleData(String msg, WiFiClient client)
{
  Serial.println("handleData..." + msg);
  if (msg.equals("query_relay_state")) {
    sendRelayStatus(client, 0);
    delay(100);
    sendRelayStatus(client, 1);
    return;
  }
  if (msg.startsWith("open:"))
  {
    int index = msg.lastIndexOf(":");
    String relayID = msg.substring(index + 1);
    Serial.println("relayID = " + relayID);
    int id = relayID.toInt();
    if (id == 0)
    {
      digitalWrite(RELAY_PIN_1, OPEN);
      RELAY_STATE[0] = 1;
      sendRelayStatus(client, 0);
    } else if (id == 1) {
      digitalWrite(RELAY_PIN_2, OPEN);
      RELAY_STATE[1] = 1;
      sendRelayStatus(client, 1);
    }

  } else if (msg.startsWith("close:")) {
    int index = msg.lastIndexOf(":");
    String relayID = msg.substring(index + 1);
    Serial.println("relayID = " + relayID);
    int id = relayID.toInt();
    if (id == 0) {
      digitalWrite(RELAY_PIN_1, CLOSE);
      RELAY_STATE[0] = 0;
      sendRelayStatus(client, 0);
    } else if (id == 1) {
      digitalWrite(RELAY_PIN_2, CLOSE);
      RELAY_STATE[1] = 0;
      sendRelayStatus(client, 1);
    }
  }
}

void loop()
{
  WiFiClient client = server.available(); // 不阻塞
  if (client)
  {
    delay(2000);
    sendRelayStatus(client, 0);
    delay(100);
    sendRelayStatus(client, 1);
    while (client.connected()) {
      delay(100);
      count ++;
      checkClient(client);
    }
  } else {
    delay(1500);
    Serial.println("IP: "+WiFi.localIP().toString()+", Waiting tcp client for connect(server_relay)");
  }
}
