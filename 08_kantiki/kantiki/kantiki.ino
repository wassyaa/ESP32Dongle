#include <DHT20.h>
#include <Wire.h>
#include <SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>

// WiFi Setting
const char* ssid     = "AP01-01";
const char* password = "xxxx";

// LINE Notify Setting
const char* host    = "notify-api.line.me";
const char* token   = "xxxx"; //Your API key
const char* message = "異常な温度を検知しました、センサーを確認してください。";

DHT20 DHT(&Wire1);

#define moterPin 25
Servo myservo;
int angle = 0;

void setup() {
  Serial.begin(115200);
  Wire1.begin(33, 32);  //  select your pin numbers here
  myservo.attach(moterPin);
  connectWiFi();
}

void loop() {
  int status = DHT.read();  //温湿を読み取り
  double T = DHT.getTemperature(); //温度をdoubleで受ける
  double H = DHT.getHumidity();    //湿度をdobleで受ける
  Serial.println(String(T) + "℃");  //温度を表示
  Serial.println(String(H) + "％");  //湿度を表示

  if (T > 60) {
    for(angle = 0; angle <= 180; angle++) {   // 0-180 right                         
    myservo.write(angle);
    }
  
   send_line(); //Line送信

  }
  delay(2000); //2秒ごとに更新
}

void connectWiFi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //Start WiFi connection
  WiFi.begin(ssid, password);

  //Check WiFi connection status
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void send_line() {
  //Access to HTTPS (SSL communication)
  WiFiClientSecure client;
  //Required when connecting without verifying the server certificate
  client.setInsecure();

  Serial.println("Try");

  //SSL connection to Line's API server (port 443: https)
  if (!client.connect(host, 443)) {
    Serial.println("Connection failed");
    return;
  }
  Serial.println("Connected");

  // Send request
  String query = String("message=") + String(message);
  String request = String("") +
    "POST /api/notify HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Authorization: Bearer " + token + "\r\n" +
    "Content-Length: " + String(query.length()) +  "\r\n" + 
    "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
    query + "\r\n";
  client.print(request);
 
  // Wait until reception is complete
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  Serial.println(line);
}