#include "esp_camera.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// 摄像头配置
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// LED控制引脚
#define LED_GPIO_NUM       4

// 定义电机控制引脚
#define IN1 2
#define IN2 14
#define IN3 15
#define IN4 13

// 替换为你的WiFi名称和密码
const char* ssid = "ZHIYI";  
const char* password = "zy12345678"; 

AsyncWebServer server(80);

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA; // 使用QVGA分辨率
    config.jpeg_quality = 15; // 适当降低JPEG质量
    config.fb_count = 2; // 使用两个帧缓冲区
  } else {
    config.frame_size = FRAMESIZE_QQVGA; // 降低分辨率
    config.jpeg_quality = 15; // 更高JPEG质量
    config.fb_count = 1; // 使用一个帧缓冲区
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
}

// 电机控制函数
void turn_right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turn_left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void forward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void backward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// 启动摄像头服务器
void startCameraServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) -> String {
    String html = "<html><body>";
    html += "<h1>ESP32 Motor Control</h1>";
    html += "<button onclick=\"fetch('/motor/on')\">前进</button>";
    html += "<button onclick=\"fetch('/motor/off')\">停止</button>";
    html += "<button onclick=\"fetch('/motor/right')\">右转</button>";
    html += "<button onclick=\"fetch('/motor/left')\">左转</button>";
    html += "<button onclick=\"fetch('/motor/back')\">后退</button>";
    html += "<h1>Camera Stream</h1>";
    html += "<img src='/camera' style='width:30%;' id='cameraFeed'>";
    html += "<script>";
    html += "function updateCameraFeed() {";
    html += "document.getElementById('cameraFeed').src = '/camera?" + String(millis()) + "';";
    html += "}";
    html += "setInterval(updateCameraFeed, 100);"; // 每100毫秒刷新一次
    html += "</script>";
    html += "</body></html>";
    return html; // 返回HTML内容
  });

  server.on("/camera", HTTP_GET, [](AsyncWebServerRequest *request) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      request->send(500, "text/plain", "Camera capture failed");
      return;
    }
    request->send_P(200, "image/jpeg", fb->buf, fb->len);
    esp_camera_fb_return(fb);
  });

  // 启动电机的处理
  server.on("/motor/on", HTTP_GET, [](AsyncWebServerRequest* request) {
    forward();
    request->send(200, "text/plain", "前进");
  });

  server.on("/motor/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    stop();
    request->send(200, "text/plain", "停止");
  });

  server.on("/motor/right", HTTP_GET, [](AsyncWebServerRequest *request) {
    turn_right();
    request->send(200, "text/plain", "右转");
  });

  server.on("/motor/left", HTTP_GET, [](AsyncWebServerRequest *request) {
    turn_left();
    request->send(200, "text/plain", "左转");
  });

  server.on("/motor/back", HTTP_GET, [](AsyncWebServerRequest *request) {
    backward();
    request->send(200, "text/plain", "后退");
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("连接中...");
  }

  Serial.println("已连接到 WiFi");
  Serial.print("IP 地址: ");
  Serial.println(WiFi.localIP());

  setupCamera(); // 初始化摄像头
  startCameraServer(); // 启动摄像头服务器
}

void loop() {
  // 不需要循环内容
}
