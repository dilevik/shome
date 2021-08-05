#include <WiFi.h>
#include <PubSubClient.h>
//#include <ArduinoJson.h>

unsigned long timer19s;
unsigned long timer10s;

const char* ssid = "torpeda";
const char* password =  "9035913991";
const char* mqttServer = "192.168.0.10";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
//IPAddress local_IP(192, 168, 0, 92);
//IPAddress gateway(192, 168, 0, 1);
//IPAddress subnet(255, 255, 255, 0);
//IPAddress primaryDNS(192, 168, 0, 1); //optional
//IPAddress secondaryDNS(192, 168, 0, 19); //optional


boolean i27, i13;
byte cds;//currentdoorstate

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espClient;
PubSubClient client(mqttServer, 1883, callback, espClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {

  unsigned long lastMqtt = 0;
  payload[length] = '\0';
  Serial.print(topic);
  Serial.print("  ");
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  Serial.println(strPayload);

  if (strTopic == "h/from/set") {
    if (strPayload == "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"TargetDoorState\",\"value\":0}") {
      digitalWrite(4, HIGH); delay(800); digitalWrite(4, LOW); Serial.print("komanda vorot otkr from homekit");
      if (digitalRead(27) == HIGH && digitalRead(13) == HIGH && millis() - timer19s < 19000) {
        delay(1000); digitalWrite(4, HIGH); delay(800); digitalWrite(4, LOW); Serial.print("komanda vorot otkr from homekit povtor");
      }
      timer19s = millis();

    }
    else if (strPayload == "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"TargetDoorState\",\"value\":1}")
    { digitalWrite(2, HIGH); delay(800); digitalWrite(2, LOW); Serial.print("komanda vorot zakr from homekit");
      if (digitalRead(27) == HIGH && digitalRead(13) == HIGH && millis() - timer19s < 19000) {
        delay(1000); digitalWrite(2, HIGH); delay(800); digitalWrite(2, LOW); Serial.print("komanda vorot zakr from homekit povtor");
      }
      timer19s = millis();
    }
  }





  if (strTopic == "/vorota/command") {
    if      (strPayload == "1") {
      digitalWrite(2, HIGH);
      delay(1000);
      digitalWrite(2, LOW);
      Serial.print("komanda vorot zakr");
    }
    else if (strPayload == "0") {
      digitalWrite(4, HIGH);
      delay(1000);
      digitalWrite(4, LOW);
      Serial.print("komanda vorot otkr");
    }
  }

  if (strTopic == "h/from/get") {
    if (strPayload == "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\"}") {
      if (digitalRead(27) == LOW && digitalRead(13) == LOW && millis()-timer19s>19000) {
        client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\",\"value\":4}");
      } else {
        String strix="{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\",\"value\":"+String(cds)+"}";
        client.publish ( "h/to/set" , strix.c_str());
      }
    }
  }
  if (strTopic == "h/from/connected") {
    //client.publish("h/to/add", "{\"name\": \"gdo\", \"service_name\": \"22\", \"service\": \"GarageDoorOpener\"}");
  }


  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p, payload, length);
  ////client8.publish("outTopic", p, length);
  // Free the memory
  free(p);
}

void setup() {
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(4, LOW);

  pinMode(27, INPUT_PULLUP);
  // digitalWrite(27,HIGH);
  pinMode(13, INPUT_PULLUP);
  // digitalWrite(13,HIGH);

  i27 = !digitalRead(27);
  i13 = !digitalRead(13);
  if (i27==1) {cds=1;}else {cds=4;}


  Serial.begin(115200);
  //   if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //  Serial.println("STA Failed to configure");
  // }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");//подключаемся пока не подключимся
  }


  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  client.setServer(mqttServer, mqttPort);
  reconnect();
timer10s=millis();
}


void loop() {
  if (millis()-timer10s>10000){
    client.publish ( "/vorota" , "online");
    timer10s=millis();
         
  }
  client.loop();
  if (!client.connect("ESP32Client", mqttUser, mqttPassword )) {
    reconnect();
  }

  if (i27 == digitalRead(27) || i13 == digitalRead(13)) {
    delay(100);
    if (i27 == digitalRead(27) || i13 == digitalRead(13)) {
      Serial.print("Poziciya vorot izmenilas ");
      if (digitalRead(27) == LOW && digitalRead(13) == LOW) {
        Serial.println("= Priotkryto"); client.publish("/vorota/sostoyanie", "3");
        if (cds==1) {//было закрыто

          client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\",\"value\":0}");
          //оповещаем что открылось(перестало быть закрытым)
          
          client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"TargetDoorState\",\"value\":0}");
          //сменили таргет чтоб появилась возможность значка "открытие"
          
           client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\",\"value\":3}");
          //показывает значек "открытие"
          
          } 
        if (cds==0) {//было открыто

                client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"TargetDoorState\",\"value\":1}");
          //сменили таргет чтоб появилась возможность значка "закрытие"
          
          client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\",\"value\":3}");
          //показывает значек "закрытие"
          
          }
      }
     
      if (digitalRead(27) == LOW && digitalRead(13) == HIGH) {
        timer19s = millis() - 19000;
        Serial.println("= Otkryto"); client.publish("/vorota/sostoyanie", "0");
        cds=0;
        client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"TargetDoorState\",\"value\":0}");
      }
      if (digitalRead(27) == HIGH && digitalRead(13) == LOW) {
        timer19s = millis() - 19000;
        Serial.println("= Zakryto"); client.publish("/vorota/sostoyanie", "1");
        cds=1;
        client.publish ( "h/to/set" , "{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"TargetDoorState\",\"value\":1}");
      }
      String strix="{\"name\":\"gdo\",\"service_name\":\"22\",\"characteristic\":\"CurrentDoorState\",\"value\":"+String(cds)+"}";
        client.publish ( "h/to/set" , strix.c_str());
      i27 = !digitalRead(27);
      i13 = !digitalRead(13);
      //delay (1000);
      Serial.print(digitalRead(27)); Serial.println(digitalRead(13));
    }

  }
}
