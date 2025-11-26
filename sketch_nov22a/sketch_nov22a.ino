#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Wifi name
#define WLAN_SSID       "Pp"
//Wifi password
#define WLAN_PASS       "123456789"

//setup Adafruit
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
//fill your username                   
#define AIO_USERNAME    "hoatran01112004"
//fill your key
#define AIO_KEY         ""

//setup MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//setup publish
Adafruit_MQTT_Publish light_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/led");
Adafruit_MQTT_Publish temp_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish humi_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humi");

//setup subcribe
Adafruit_MQTT_Subscribe light_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led", MQTT_QOS_1);
Adafruit_MQTT_Subscribe temp_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/temp", MQTT_QOS_0);
Adafruit_MQTT_Subscribe humi_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/humi", MQTT_QOS_0);

int led_counter = 0;
int led_status = HIGH;
String Buffer = ""; 

void lightcallback(char* value, uint16_t len){
  if(value[0] == '0') Serial.print('a');
  if(value[0] == '1') Serial.print('A');
}

void setup() {
  // put your setup code here, to run once:
  //set pin 2,5 as OUTPUT
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  //set busy pin HIGH
  digitalWrite(5, HIGH);

  Serial.begin(115200);

  //connect Wifi
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  //subscribe light feed
  light_sub.setCallback(lightcallback);
  mqtt.subscribe(&light_sub);

  //connect MQTT
  while (mqtt.connect() != 0) { 
    mqtt.disconnect();
    delay(500);
  }

  //finish setup, set busy pin LOW
  digitalWrite(5, LOW);
}
int counter = 0;
void loop() {
  // put your main code here, to run repeatedly:

  //receive packet
  mqtt.processPackets(1);
  
  //read serial
  int temp = 0;
  while (Serial.available()){
    // int msg = Serial.read();
    // if(msg == 'o') Serial.print('O');
    // else if(msg == 'a') light_pub.publish(0);
    // else if(msg == 'A') light_pub.publish(1);
    char c = Serial.read();
    //Serial.println(c);
    temp = 1;

     if (c == '!') {
      Buffer = "";  // Reset buffer khi bắt đầu
    }
    
    // Thêm ký tự vào buffer
    else if (c == '#'){
      float a = 0;
      float b = 0.1;
      bool hit = 0;
      for (int i = 0; i < Buffer.length(); i++) {
          char c = Buffer[i];

          if (c >= '0' && c <= '9') {   // make sure it's a digit
              if (hit ==1){
                a = a + (c - '0')*b;
                b = b*0.1;
              }
              else a = a * 10 + (c - '0');   // convert char to int
          }
          else if (c == '.') hit = 1;
      }
      temp_pub.publish(a);
      continue;
    }
    else Buffer += c;

  }
  if (temp==1){
    led_counter = 0;
    //toggle LED
    if(led_status == HIGH) led_status = LOW;
    else led_status = HIGH;

    digitalWrite(2, led_status);

    // temp_pub.publish(12);
  }



  led_counter++;
  if(led_counter == 100){
    // every 1s
    led_counter = 0;
    //toggle LED
    if(led_status == HIGH) led_status = LOW;
    else led_status = HIGH;

    // digitalWrite(2, led_status);
    // Serial.println("ddd");
  }
  

  delay(10);
}