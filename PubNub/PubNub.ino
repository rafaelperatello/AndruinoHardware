
#include <SPI.h>
#include <Ethernet.h>
#include <PubNub.h>
#include <ArduinoJson.h>

const int pinLed1 = 36;
const int pinLed2 = 38;
const int pinLed3 = 2;

const int pinBt1 = 24;
const int pinBt2 = 26;
const int pinBt3 = 28;

// Some Ethernet shields have a MAC address printed on a sticker on the shield;
// fill in that address here, or choose your own at random:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const int subLedPin = 8;

char pubkey[] = "pub-c-24a86643-ecbc-40b3-ad4f-18f4b24f568e";
char subkey[] = "sub-c-27383722-e188-11e5-ba64-0619f8945a4f";
char channel[] = "ANDRUINO";
char uuid[] = "xxxxxxxx-xxxx-4444-9999-xxxxxxxxxxxx";

void random_uuid() {
  randomSeed(analogRead(4) + millis() * 1024);
  snprintf(uuid, sizeof(uuid), "%04lx%04lx-%04lx-4444-9999-%04lx%04lx%04lx",
           random(0x10000), random(0x10000), random(0x10000),
           random(0x10000), random(0x10000), random(0x10000));
}

unsigned long start_time;
unsigned long timeout_time = 4000;

void setup()
{
  pinMode(subLedPin, OUTPUT);
  digitalWrite(subLedPin, LOW);

  pinMode(pinBt1, INPUT);
  pinMode(pinBt2, INPUT);
  pinMode(pinBt3, INPUT);

  pinMode(pinLed1, OUTPUT);
  pinMode(pinLed2, OUTPUT);
  pinMode(pinLed3, OUTPUT);

  digitalWrite(pinLed1, LOW);
  digitalWrite(pinLed2, LOW);
  digitalWrite(pinLed3, LOW);


  Serial.begin(115200);
  Serial.println("Serial set up");

  while (!Ethernet.begin(mac)) {
    Serial.println("Ethernet setup error");
    delay(1000);
  }
  Serial.println("Ethernet set up");

  PubNub.begin(pubkey, subkey);
  random_uuid();
  PubNub.set_uuid(uuid);
  Serial.println("PubNub set up");

  start_time = millis();
}

void loop()
{
  Ethernet.maintain();

  //subscribe
  PubSubClient *client;

  Serial.println("waiting for a message (subscribe)");
  client = PubNub.subscribe(channel);

  if (!client) {
    Serial.println("subscription error");
    delay(1000);
    return;
  }

  String receivedData = "";

  Serial.print("Received: ");
  while (client->wait_for_data() ) {
    char c = client->read();
    receivedData += c;
    Serial.print(c);
  }

  client->stop();
  Serial.println();

  Serial.println("string data");
  Serial.println(receivedData);

  StaticJsonBuffer<1000> arrayBuffer;
  StaticJsonBuffer<200> jsonBuffer;

  JsonArray& root = arrayBuffer.parseArray(receivedData);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseArray() failed");
    return;
  }
  Serial.println();
  Serial.println("print root");
  root.printTo(Serial);

  for (JsonArray::iterator it = root.begin(); it != root.end(); ++it) {
    String value = *it;
    Serial.println();
    Serial.println("iterator: ");
    Serial.println(value);

    JsonObject& json = *it;
    Serial.println();
    Serial.println("print json");
    json.printTo(Serial);

    if (json.containsKey("led1")) {
      int value1 = json["led1"];
      Serial.println();
      Serial.println("Led1");
      Serial.println(value1);
      digitalWrite(pinLed1, value1);
    }

    if (json.containsKey("led2")) {
      int value2 = json["led2"];
      Serial.println();
      Serial.println("Led2");
      Serial.println(value2);
      digitalWrite(pinLed2, value2);
    }

    if (json.containsKey("led3")) {
      int value3 = json["led3"];
      Serial.println();
      Serial.println("Led3");
      Serial.println(value3);
      analogWrite(pinLed3, value3);
    }
  }
  
  delay(1000);
  
  
  //Publish status
  if ((millis() - start_time) > timeout_time) {
    start_time = millis();

    Serial.println("publishing bt status");
    //  publish
    EthernetClient *pClient;

    StaticJsonBuffer<200> jsonCreatorBuffer;
    JsonObject& rootCreator = jsonCreatorBuffer.createObject();

    bool button1State = digitalRead(pinBt1);
    bool button2State = digitalRead(pinBt2);
    bool button3State = digitalRead(pinBt3);
    
    //bt1
    if (button1State){
      rootCreator["bt1"] = "1";
    }
    else{
      rootCreator["bt1"] = "0";
    }
  
    //bt2
    if (button2State){
      rootCreator["bt2"] = "1";
    }
    else{
      rootCreator["bt2"] = "0";
    }
      
    //bt3
    if (button3State){
      rootCreator["bt3"] = "1";
    }
    else{
      rootCreator["bt3"] = "0";
    }

    char buffer[256];
    rootCreator.printTo(buffer, sizeof(buffer));
  
    Serial.print("publishing message: ");
    Serial.println(buffer);
    rootCreator.printTo(Serial);
    
    pClient = PubNub.publish(channel, buffer);
    if (!pClient) {
      Serial.println("publishing error");
    } else {
      pClient->stop();
    }
  }
}

