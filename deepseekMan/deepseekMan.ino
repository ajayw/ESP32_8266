#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>

// WiFi 连接信息
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// DeepSeek WebSocket 服务器
const char* websocketServer = "your.deepseek.server";
const int websocketPort = 443;

WebSocketsClient webSocket;

// OLED 显示
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// I2S0 配置（INMP441 麦克风）
#define I2S_MIC_WS  15
#define I2S_MIC_SCK 14
#define I2S_MIC_SD  32
#define I2S_MIC_PORT I2S_NUM_0

// I2S1 配置（MAX98357 扬声器）
#define I2S_SPK_WS  25
#define I2S_SPK_SCK 26
#define I2S_SPK_DOUT 22
#define I2S_SPK_PORT I2S_NUM_1

bool isConnected = false;
bool isListening = false;

// WiFi 连接
void connectWiFi() {
    Serial.print("连接 WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi 连接成功！");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.localIP());
}

// WebSocket 连接
void connectWebSocket() {
    Serial.println("连接 WebSocket...");
    webSocket.beginSSL(websocketServer, websocketPort, "/ws");
    webSocket.onEvent(webSocketEvent);
    webSocket.setAuthorization("Bearer YOUR_ACCESS_TOKEN");
    webSocket.setReconnectInterval(5000);
}

// WebSocket 事件
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("WebSocket 连接成功！");
            isConnected = true;
            sendHelloMessage();
            break;
        case WStype_DISCONNECTED:
            Serial.println("WebSocket 断开！");
            isConnected = false;
            break;
        case WStype_TEXT:
            processJsonMessage((char*)payload);
            break;
        case WStype_BIN:
            Serial.println("收到 TTS 语音数据");
            playAudio(payload, length);
            break;
    }
}

// 发送 "hello" 握手消息
void sendHelloMessage() {
    DynamicJsonDocument doc(256);
    doc["type"] = "hello";
    doc["version"] = 1;
    doc["transport"] = "websocket";
    
    JsonObject audioParams = doc.createNestedObject("audio_params");
    audioParams["format"] = "opus";
    audioParams["sample_rate"] = 16000;
    audioParams["channels"] = 1;
    audioParams["frame_duration"] = 60;

    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.sendTXT(jsonString);
}

// OLED 显示文本
void displayText(String text) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

// 播放音频（MAX98357）
void playAudio(uint8_t *data, size_t length) {
    i2s_write(I2S_SPK_PORT, data, length, NULL, 100);
}

void setup() {
    Serial.begin(115200);
    connectWiFi();
    connectWebSocket();

    // 初始化 OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED 初始化失败！");
        while (1);
    }
    display.clearDisplay();
}

void loop() {
    webSocket.loop();
}
