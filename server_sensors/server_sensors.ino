#include <ESP8266WiFi.h>
#include "DHT.h"

// 传感器(DHT11温湿度 MQ2气体检测)模块代码 选择开发板D1(Retired)

#define DHTPIN  D2    // what digital pin we're connected to

String arduinoID = "01";

#define DHTTYPE DHT11   // DHT 11

#define PORT 9000

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "alonebo";
const char* password = "976447044";

WiFiServer server(PORT);

void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.println("DHT11 init...!");

  dht.begin();

  Serial.println("Pin init...!");

  pinMode(BUILTIN_LED, OUTPUT);

  Serial.printf("Connecting to %s ", ssid);
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
// id     传感器id 上传/下发 data1 data2 校验位
String tmpData = "";
String humData = "";
String mq2Data = "";

int setData()
{
  float t = dht.readTemperature(); // 读取温度
  if (isnan(t)) {
    Serial.println("Failed to read from DHT Temp sensor!");
    return -1;
  }
  delay(100);
  float h = dht.readHumidity(); // 读取 湿度
  if (isnan(h)) {
    Serial.println("Failed to read from DHT Hum sensor!");
    return -1;
  }
  Serial.print("Tmp: ");
  Serial.println(t);
  Serial.print("Hum: ");
  Serial.println(h);
  String strT = String(t, 2);
  String strH = String(h, 2);
  tmpData = String(arduinoID + "_10_10_"  + strT);
  humData = String(arduinoID + "_20_10_" + strH);

  int val = analogRead(A0);// 读取MQ2
  String strMQ = String(val, DEC);
  mq2Data = String(arduinoID + "_30_10_" + strMQ);
  Serial.print("MQ-2: ");
  Serial.println(strMQ);
  return 0;
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

void blinkLed()
{
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);

}

int sendData(WiFiClient client)
{

  client.print(tmpData);
  blinkLedFast();
  delay(500);
  client.print(humData);
  delay(500);
  client.print(mq2Data);
  delay(500);
  blinkLedFast();
  return 0;
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
      sendData(client); // 发送数据
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

void handleData(String msg, WiFiClient client)
{
  Serial.println("handleData..." + msg);
  Serial.println("this is sensor module, the recived data was discard。");
}


void loop()
{
  WiFiClient client = server.available(); // 不阻塞
  if (client)
  {
    while (client.connected()) {
      delay(2000);
      setData();
      checkClient(client);
    }

  } else {
    delay(1500);
    Serial.println("IP: "+WiFi.localIP().toString()+", Waiting tcp client for connect(server_sensors)");
  }
}
