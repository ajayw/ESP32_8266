#include <WiFi.h>
#include <PubSubClient.h>  //A client library for MQTT messaging.
#include <DHT.h>
#include <ArduinoJson.h>

#define LED 22              // LED引脚
#define DHTPIN 4            // Defines pin number to which the sensor is connected
#define DHTTYPE DHT11       // DHT 11
DHT dht(DHTPIN, DHTTYPE);   // Creats a DHT object

/* 连接WIFI SSID和密码 */
#define WIFI_SSID "ZHIYI"
#define WIFI_PASSWD "zy12345678"

/* 设备的三元组信息*/
#define PRODUCT_KEY "k15j9kWxg8H"
#define DEVICE_NAME "TempHumi"
#define DEVICE_SECRET "3012e866383a1931cb2bc963f6dfd15e"
#define REGION_ID "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER "iot-06z00bpscn7cdjt.mqtt.iothub.aliyuncs.com"
#define MQTT_PORT 1883
#define MQTT_USRNAME DEVICE_NAME "&" PRODUCT_KEY

#define CLIENT_ID "k15j9kWxg8H.TempHumi|securemode=2,signmethod=hmacsha256,timestamp=1715070823046|"
#define MQTT_PASSWD "dbb0a917f61b85032abc363b7c8259f11d3fffaef8fbca179ff8ffcf9a587a7e"

//宏定义订阅主题
#define ALINK_BODY_FORMAT "{\"id\":\"TempHumi\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

const char* topic = "/sys/k15j9kWxg8H/TempHumi/thing/service/property/set";
unsigned long lastMs = 0;
extern String WiFiAddr = "";
float HumiValue;
float TempValue;

WiFiClient espClient;
PubSubClient client(espClient);

//连接wifi,获取MAC地址
void wifiInit() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  WiFiAddr = WiFi.localIP().toString();
  Serial.println("' to connect");
  // 获取MAC地址
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  // 打印MAC地址
  Serial.printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

//mqtt连接
void mqttCheckConnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT Server ...");
    if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD)) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.print("MQTT Connect err:");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

//上传温湿度
void mqttIntervalPost() {
  char param[32];
  char jsonBuf[128];

  //upload humidity
  HumiValue = dht.readHumidity();
  sprintf(param, "{\"RoomHumidity\":%0.1f}", HumiValue);
  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
  Serial.println(jsonBuf);
  bool b = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  // if(b){
  //   Serial.println("publish Humidity success");
  // }else{
  //   Serial.println("publish Humidity fail");
  // }

  // Upload temperature
  TempValue = dht.readTemperature();
  sprintf(param, "{\"RoomTemp\":%0.2f}", TempValue);
  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
  Serial.println(jsonBuf);
  bool c = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  // if(c){
  //   Serial.println("publish Temperature success");
  // }else{
  //   Serial.println("publish Temperature fail");
  // }
}

//回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);                // 打印主题信息Message arrived [/sys/k15j9kWxg8H/TempHumi/thing/service/property/set]
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);   // 打印主题内容{"method":"thing.service.property.set","id":"704369436","params":{"LightSwitch":1},"version":"1.0.0"}
  }
  Serial.println();

  DynamicJsonDocument doc(1024);                 //创建了一个名为 doc 的动态 JSON 文档
  deserializeJson(doc, String((char*)payload));  //从一个名为 payload 的数据中解析 JSON 数据并将其填充到 doc 中
  if (doc["params"].containsKey("LightSwitch")) {
    Serial.println("GOT LightSwitch CMD");
    digitalWrite(LED, doc["params"]["LightSwitch"]);
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  wifiInit();
  client.setServer(MQTT_SERVER, MQTT_PORT); /* 连接MQTT服务器 */
  client.setCallback(callback);             //设定回调方式，当ESP32收到订阅消息时会调用此方法
  digitalWrite(LED, LOW);
}

void loop() {
  if (millis() - lastMs >= 5000) {
    lastMs = millis();
    mqttCheckConnect();
    mqttIntervalPost();
  }
  client.loop();
  // delay(2000);
}
