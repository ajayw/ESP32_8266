#include <WiFi.h>

const char* ssid = "YOUR_SSID";       // 替换为你的 Wi-Fi 名称
const char* password = "YOUR_PASSWORD"; // 替换为你的 Wi-Fi 密码

#include <HTTPClient.h>
#include <ArduinoJson.h> // 用于解析 JSON 响应

void get_ai_response(const String& user_input) {
    HTTPClient http;
    http.begin("https://api.deepseek.com/chat"); // DeepSeek API URL
    http.addHeader("Content-Type", "application/json");

    // 构造 JSON 请求体
    String post_data = "{\"input\": \"" + user_input + "\"}";
    int httpResponseCode = http.POST(post_data); // 发送 POST 请求

    if (httpResponseCode == 200) { // 如果请求成功
        String response = http.getString(); // 获取 API 响应
        Serial.println("Response: " + response);

        // 解析 JSON 响应
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        String ai_reply = doc["response"]; // 假设 API 返回的字段是 "response"
        Serial.println("AI Reply: " + ai_reply);

        // 调用 TTS 播放 AI 回复
        play_audio(ai_reply);
    } else {
        Serial.println("Error: " + String(httpResponseCode));
    }
    http.end(); // 关闭连接
}

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

AudioGeneratorWAV *audio;
AudioFileSourcePROGMEM *file;
AudioOutputI2S *out;

void play_audio(const String& text) {
    // 将文本转换为音频数据（这里需要预先生成音频文件）
    const uint8_t audio_data[] PROGMEM = { /* 音频数据 */ };
    file = new AudioFileSourcePROGMEM(audio_data, sizeof(audio_data));
    out = new AudioOutputI2S();
    audio = new AudioGeneratorWAV();

    // 设置 I2S 引脚（BCLK, LRC, DIN）
    out->SetPinout(25, 26, 27);
    audio->begin(file, out);

    // 播放音频
    while (audio->isRunning()) {
        if (!audio->loop()) audio->stop();
    }
}

const uint8_t audio_data_1[] PROGMEM = { /* 音色 1 的音频数据 */ };
const uint8_t audio_data_2[] PROGMEM = { /* 音色 2 的音频数据 */ };

void set_voice(int voice_id) {
    if (voice_id == 1) {
        play_audio(audio_data_1, sizeof(audio_data_1));
    } else if (voice_id == 2) {
        play_audio(audio_data_2, sizeof(audio_data_2));
    }
}

void setup() {
    Serial.begin(115200); // 初始化串口通信
    WiFi.begin(ssid, password); // 连接 Wi-Fi

    // 等待 Wi-Fi 连接
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    String user_input = "Hello, how are you?";
    get_ai_response(user_input); // 获取 AI 回复

    set_voice(1); // 设置音色
    play_audio("This is a custom voice response."); // 播放回复
}

void loop() {
    // 主循环代码
}

