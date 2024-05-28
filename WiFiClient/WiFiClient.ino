#if defined(ARDUINO_ARCH_ESP8266)  // 如果使用了ESP8266平台
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)  // 如果使用了ESP32平台
#include <WiFi.h>
#endif
#include "AliyunMqtt.h"
#include "DHT.h"
#define debugState true  // 串口调试打印状态，true--开启调试打印，false--关闭调试打印
/* 连接WIFI SSID和密码 */
#define WIFI_SSID "ZHIYI"
#define WIFI_PASSWORD "zy12345678"

/* 线上环境域名和端口号*/
#define MQTT_PORT 1883

/* 三元组信息*/
#define PRODUCT_KEY "a1w1mbaNjWV"
#define DEVICE_NAME "TempHumi"
#define DEVICE_SECRET "448c1db784fe8095e468537c1698ce0d"

#define LED_PIN 15         // Specify the on which is your LED
#define DHTPIN 4           // Defines pin number to which the sensor is connected
#define DHTTYPE DHT11      // DHT 11
#define ckeckMqttConnectTime 10000  // 检查MQTT连接的间隔时间，默认为10秒
DHT dht(DHTPIN, DHTTYPE);  // Creats a DHT object
extern AsyncMqttClient mqttClient;
AliyunMqtt aliyunMqtt;

//宏定义发布/订阅主题
#define ALINK_BODY_FORMAT "{\"id\":\"TempHumi\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
#define ALINK_TOPIC_PROP_SET "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"

extern String WiFiAddr = "";
float TempValue;
float HumiValue;
int lightSwitch;
bool MqttConnectState = false;
uint32_t agoTime = 0;
uint32_t CurrentTime;
//WiFi连接函数
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.onEvent(WiFiEvent);
  while (WL_CONNECTED != WiFi.status()) {  // 判断WIFI的连接状态
    delay(500);
#if debugState
    Serial.print(".");
#endif
  }
}

//MQTT连接函数
void checkMqttAndReconnect() {
  static uint8_t mqttConnectStateMachine = 0;  // MQTT连接的状态机
  static uint32_t nowTime = 0;                 // 检查时间
  switch (mqttConnectStateMachine) {
    // 检查MQTT的连接状态
    case 0:
      {
        if (MqttConnectState == false) {  // 如果当前MQTT的连接状态为已断开连接
          mqttClient.disconnect(true);    // 断开之前的连接
          mqttClient.connect();           // 连接MQTT服务器
          mqttConnectStateMachine = 1;    // MQTT连接中，状态机=1，重连中
          nowTime = millis();             // 更新检查时间
#if debugState
          Serial.println("MQTT服务器已断开连接！开始尝试重连中......");
#endif
        }
      }
      break;
    // MQTT重连中
    case 1:
      {
        if ((MqttConnectState == true) || millis() - nowTime > ckeckMqttConnectTime) {  // 如果当前MQTT的连接状态为已连接或者超过重连时间
          mqttConnectStateMachine = 0;                                                  // 状态机=0，回到检查中
        }
      }
      break;
    default:
      break;
  }
}
/**
函数功能：WiFi库监听连接状态事件
默认参数：event ：0/1/2/3/4/5/6/7
          SYSTEM_EVENT_STA_GOT_IP:连接上WIFI后获得IP地址
          SYSTEM_EVENT_STA_DISCONNECTED：未成功连接WiFi标志
*/
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      break;
  }
}

/**
断开mqtt回调
*/
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {

  Serial.println("Disconnected from MQTT.");
  MqttConnectState = false;  // 当前MQTT的连接状态为已断开连接
  switch (reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
      Serial.println("Reason: TCP disconnected");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
      Serial.println("Reason: MQTT unacceptable protocol version");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
      Serial.println("Reason: MQTT identifier rejected");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
      Serial.println("Reason: MQTT server unavailable");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
      Serial.println("Reason: MQTT malformed credentials");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
      Serial.println("Reason: MQTT not authorized");
      break;
    default:
      Serial.println("Reason: Unknown reason");
      break;
  }
}
/**
连接mqtt成功回调
*/
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  MqttConnectState = true;
  // uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  // Serial.print("Subscribing at QoS 2, packetId: ");
  // Serial.println(packetIdSub);
  // mqttClient.publish("test/lol", 0, true, "test 1");
  // Serial.println("Publishing at QoS 0");
  // uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPub1);
  // uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  // Serial.print("Publishing at QoS 2, packetId: ");
  // Serial.println(packetIdPub2);
}

//上报温湿度数据到平台
void getTempHumi() {
  if (MqttConnectState == true) {
    TempValue = dht.readTemperature();
    HumiValue = dht.readHumidity();
// #if debugState
//     Serial.print("TempValue:");
//     Serial.print(TempValue);
//     Serial.print("--HumiValue:");
//     Serial.println(HumiValue);
// #endif
    if (!isnan(TempValue) && !isnan(HumiValue)) {
      char payload[48];
      char jsonBuf[128];
      CurrentTime = millis();
      if(CurrentTime-agoTime>3000)
      {
        snprintf(payload, sizeof(payload), "{\"temperature\":%0.2f,\"humidity\":%0.1f}", TempValue, HumiValue);
        snprintf(jsonBuf, sizeof(jsonBuf),ALINK_BODY_FORMAT, payload);
        mqttClient.publish(ALINK_TOPIC_PROP_POST, 1, true, jsonBuf);
        agoTime = millis();
        Serial.println(jsonBuf);
      }
    } else {
      Serial.println("Failed to read temperature and humidity data.");
    }
  }
}

/**
订阅成功回调
*/
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}
/**
取消订阅之后回调
*/
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

/*
char* topic: 表示接收到的消息所属的主题。
char* payload: 表示接收到的消息的内容，以字符串形式表示。
AsyncMqttClientMessageProperties properties: 表示接收到的消息的属性，包括 QoS、重复标志（dup）、保留标志（retain）等。
size_t len: 表示接收到的消息的长度。
size_t index: 如果消息被分片发送，则 index 表示当前分片的索引。
size_t total: 如果消息被分片发送，则 total 表示消息的总分片数。
订阅主题后得到的内容为：{\"id\":\"1\",\"version\":\"1.0\",\"params\":{\"LightSwitch\":0}}，
*/
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);

  // 解析 JSON 数据
  DynamicJsonDocument doc(512);  // JSON 文档
  deserializeJson(doc, payload);

  // 检查是否包含LightSwitch
  if (doc.containsKey("params") && doc["params"].containsKey("envTempOnOff")) {
    // 提取 LightSwitch 的值
    lightSwitch = doc["params"]["envTempOnOff"];
    if (lightSwitch == 0) {
      // 灯关闭状态
      digitalWrite(LED_PIN, LOW);
    } else if (lightSwitch == 1) {
      // 灯打开状态
      digitalWrite(LED_PIN, HIGH);
    }
  }
}
// /**
// 发布主题成功回调
// */
void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);

  WiFi.disconnect(true);  // 断开当前WIFI连接
  WiFi.mode(WIFI_STA);    // 设置WIFI模式为STA模式
  connectToWifi();        // 开始连接WIFI
  /*
  setDeviceCertificate：封装的连接MQTT参数整合关键函数调用；
  传入参数：产品KEY，设备名称，设备秘钥；
  */
  aliyunMqtt.setDeviceCertificate(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);

  mqttClient.setKeepAlive(60);
  mqttClient.setMaxTopicLength(1024);//设置最大消息长度
  mqttClient.connect();
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
}

void loop() {
  if ((WL_CONNECTED == WiFi.status()) && (MqttConnectState == false)) checkMqttAndReconnect();
  getTempHumi();
}
