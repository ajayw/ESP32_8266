#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <Preferences.h>  // 用于存储灯光状态
#include "HomeSystem.h"
// WiFi 连接信息
const char* ssid = "iQOO12";
const char* password = "womima2013.";

// OpenWeatherMap API 信息
const char* apiKey = "a32b831995d66f7c5563e9d2a1ed5278";
const char* cityID = "1795565";  
String weatherUrl;  // 动态生成

// NTP 服务器
const char* ntpServer = "pool.ntp.org";
long gmtOffset_sec = 8 * 3600;  
int daylightOffset_sec = 0;

// 触摸按钮坐标（灯开关）
#define BUTTON_X 110
#define BUTTON_Y 180
#define BUTTON_RADIUS 30

bool lightState = false;  
bool lastTouchState = false;  // 记录上次触摸状态
TFT_eSPI tft = TFT_eSPI();
Preferences preferences;  // ESP32 存储对象

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);

    uint16_t calData[5] = { 481, 3371, 327, 3366, 5 };
    tft.setTouch(calData);

    // 读取上次存储的灯光状态
    preferences.begin("homePanel", false);
    lightState = preferences.getBool("lightState", false);

    // 连接 WiFi
    connectWiFi();

    // 生成天气 API URL
    weatherUrl = "http://api.openweathermap.org/data/2.5/weather?id=" + String(cityID) + "&appid=" + String(apiKey) + "&units=metric&lang=en";

    // 获取天气数据
    getWeatherData();
    delay(3000);

    // 配置 NTP 时间
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // 绘制 UI
    drawUI();
    // uint16_t myColor = tft.color565(128, 64, 255);  // 淡紫色，myColor填入最后参数可得颜色
    fillBtn();
    tft.fillCircle(BUTTON_X-60, BUTTON_Y, BUTTON_RADIUS, TFT_RED);
}
void fillBtn(){
    tft.drawBitmap(170, 34, tempIcon, 64, 64 , TFT_CYAN, TFT_BLACK);
    tft.fillCircle(BUTTON_X-60, BUTTON_Y, BUTTON_RADIUS+4, TFT_BROWN);
    tft.fillRect(BUTTON_X-60, BUTTON_Y-BUTTON_RADIUS-4, 60, 69, TFT_BROWN);
    tft.fillCircle(BUTTON_X, BUTTON_Y, BUTTON_RADIUS+4, TFT_BROWN);
}
void loop() {
    displayTime();

    // 每 30 分钟更新一次天气
    static unsigned long lastWeatherUpdate = 0;
    if (millis() - lastWeatherUpdate > 1800000) {
        getWeatherData();
        lastWeatherUpdate = millis();
    }

    checkTouch();
}

// 连接 WiFi（加超时）
void connectWiFi() {
    WiFi.begin(ssid, password);
    tft.setCursor(10, 10);
    tft.print("连接WiFi...");
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi 连接成功!");
        tft.fillScreen(TFT_BLACK);
    } else {
        Serial.println("\nWiFi 连接失败!");
        tft.setCursor(10, 40);
        tft.print("WiFi 失败");
    }
}

// 获取天气数据
void getWeatherData() {
    HTTPClient http;
    http.begin(weatherUrl);
    int httpCode = http.GET();

    if (httpCode == 200) {  // 确保返回 HTTP 200
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        
        String weatherDesc = doc["weather"][0]["main"].as<String>();
        float temperature = doc["main"]["temp"];
        float humidity = doc["main"]["humidity"];
        Serial.println(weatherDesc);
        // tft.fillRect(10, 50, 200, 40, TFT_BLACK);  // 清除旧天气信息
        // tft.setCursor(10, 50);
        // tft.printf("天气: %s", weatherDesc.c_str());
        // 根据天气描述选择相应图标
        if (weatherDesc.indexOf("Clear") >= 0) {
            // 晴天
            tft.drawBitmap(40, 34, sunIcon, 66, 64 , TFT_YELLOW, TFT_BLACK);
        } else if (weatherDesc.indexOf("Clouds") >= 0) {
            // 多云
            tft.drawBitmap(40, 34, cloudIcon, 72, 64 , TFT_MAGENTA, TFT_BLACK);
        } else if (weatherDesc.indexOf("Rain") >= 0) {
            // 雨天
            tft.drawBitmap(40, 34, rainIcon, 64, 64 , TFT_BLUE, TFT_BLACK);
        } else {
            // 默认图标
            tft.drawBitmap(40, 34, cloudIcon, 64, 64 , TFT_MAGENTA, TFT_BLACK);
        }

        tft.setCursor(234, 60);
        tft.printf("%.1f C", temperature);
        tft.setCursor(234, 80);
        tft.printf("%.1f%%",humidity);
    }
    http.end();
}

// 绘制 UI
void drawUI() {
    drawButton(lightState);
}

// 显示时间
void displayTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("获取时间失败");
        return;
    }

    char timeStr[16];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    tft.fillRect(170, 10, 320, 20, TFT_BLACK);  
    tft.setCursor(170, 10);
    tft.print(timeStr);

    char dateStr[16];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    tft.fillRect(10, 10, 160, 20, TFT_BLACK);  
    tft.setCursor(10, 10);
    tft.print(dateStr);
}

// 画灯光开关按钮
void drawButton(bool state) {
    fillBtn();
    uint16_t color = state ? TFT_GREEN : TFT_RED;
    uint16_t btnX = state ? BUTTON_X : BUTTON_X-60;
    tft.fillCircle(btnX, BUTTON_Y, BUTTON_RADIUS, color);
}

// 检测触摸
void checkTouch() {
    uint16_t x, y;
    bool touchState = tft.getTouch(&x, &y);
    if (touchState && !lastTouchState) {
        if ((x > BUTTON_X-60-20) && (x < BUTTON_X+20) && (y > BUTTON_Y-20) && (y < BUTTON_Y+20)) {
            lightState = !lightState;  // 切换状态
            preferences.putBool("lightState", lightState);  // 存储状态
            drawButton(lightState);
            Serial.println(lightState ? "灯光打开" : "灯光关闭");
        }
    }
    lastTouchState = touchState;
}
