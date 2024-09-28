#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// 定义电机控制引脚
#define IN1 2
#define IN2 14
#define IN3 15
#define IN4 13

const char* ssid = "ZHIYI";  
const char* password = "zy12345678"; 

void turn_right(){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);
  Serial.println("右转指令已发送");
}

void turn_left(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);
  Serial.println("左转指令已发送");
}

void forward(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);
  Serial.println("前进指令已发送");
  delay(100);  // 确保电机能有时间反应
}

void backward(){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);
  Serial.println("后退指令已发送");
}

void stop(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  Serial.println("停止指令已发送");
}
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 设置根路径
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", 
    "<html>"
    "<head>"
    "<title>Motor Control</title>"
    "<style>"
    "body { font-family: 'Arial', sans-serif; background: linear-gradient(to right, #000000, #434343); margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; overflow: hidden; position: relative; }"
    ".container { text-align: center; background: rgba(255, 255, 255, 0.2); border-radius: 15px; box-shadow: 0 8px 30px rgba(0,0,0,0.5); padding: 40px; display: flex; flex-direction: column; align-items: center; }"
    "h1 { color: #fff; font-size: 2.5em; margin-bottom: 20px; }"
    ".button-row { display: flex; justify-content: center; margin: 10px 0; }"
    "button { padding: 15px 20px; font-size: 18px; margin: 10px; border: none; border-radius: 8px; cursor: pointer; transition: transform 0.2s, background-color 0.3s; width: 150px; }"
    ".btn1 { background-color: #a8d8ea; color: black; }"
    ".btn1:hover { background-color: #90d5e2; transform: scale(1.05); }"
    ".btn2 { background-color: #ffd3b6; color: black; }"
    ".btn2:hover { background-color: #ffb6a6; transform: scale(1.05); }"
    ".btn3 { background-color: #ff677d; color: white; }"
    ".btn3:hover { background-color: #ff4f69; transform: scale(1.05); }"
    ".btn4 { background-color: #d8e2dc; color: black; }"
    ".btn4:hover { background-color: #c8d6d8; transform: scale(1.05); }"
    "@keyframes fall { 0% { transform: translateY(0) rotate(0deg); } 100% { transform: translateY(100vh) rotate(360deg); } }"
    ".petal { position: absolute; top: -10%; opacity: 0.8; animation: fall linear infinite; }"
    ".petal::before { content: ''; width: 10px; height: 20px; background-color: rgba(255, 182, 193, 0.7); border-radius: 50%; position: absolute; top: 0; left: 0; transform: rotate(45deg); }"
    ".petal::after { content: ''; width: 10px; height: 20px; background-color: rgba(255, 182, 193, 0.7); border-radius: 50%; position: absolute; top: 0; left: 0; transform: rotate(-45deg); }"
    "</style>"
    "<script>"
    "function createPetal() {"
    "  const petal = document.createElement('div');"
    "  petal.className = 'petal';"
    "  petal.style.left = Math.random() * 100 + 'vw';"
    "  petal.style.animationDuration = (Math.random() * 5 + 5) + 's';"
    "  petal.style.transform = 'translateY(-50%) rotate(' + (Math.random() * 360) + 'deg)';"
    "  document.body.appendChild(petal);"
    "  petal.addEventListener('animationend', () => { petal.remove(); });"
    "}"
    "setInterval(createPetal, 2000);"
    "</script>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<h1>Motor Control</h1>"
    "<div class='button-row'>"
    "<button class='btn1' onclick=\"fetch('/motor/on')\">foward</button>"
    "</div>"
    "<div>"
    "<button class='btn1' onclick=\"fetch('/motor/left')\">Turn_left</button>"
    "<button class='btn2' onclick=\"fetch('/motor/off')\">stop</button>"
    "<button class='btn1' onclick=\"fetch('/motor/right')\">Turn_right</button>"
    "</div>"
    "<div>"
    "<button class='btn1' onclick=\"fetch('/motor/back')\">backward</button>"
    "</div>"
    "<div class='button-row'>"
    "<button class='btn2' onclick=\"fetch('/motor/mode1')\">Mode1</button>"
    "<button class='btn3' onclick=\"fetch('/motor/mode2')\">Mode2</button>"
    "<button class='btn4' onclick=\"fetch('/motor/mode3')\">Mode3</button>"
    "</div>"
    "</div>"
    "</body>"
    "</html>");
  });

  // 启动电机的处理
  server.on("/motor/on", HTTP_GET, [](AsyncWebServerRequest *request){
    forward();
    Serial.println("电机启动 ： 前进");
    request->send(200, "text/plain", "foward");
  });

  // 停止电机的处理
  server.on("/motor/off", HTTP_GET, [](AsyncWebServerRequest *request){
    stop();
    Serial.println("电机启动 ： 停止");
    request->send(200, "text/plain", "stop");
  });

  // 右转处理
  server.on("/motor/right", HTTP_GET, [](AsyncWebServerRequest *request){
    turn_right();
    Serial.println("电机启动 ： 右转");
    request->send(200, "text/plain", "turn_right");
  });

  // 左转处理
  server.on("/motor/left", HTTP_GET, [](AsyncWebServerRequest *request){
    turn_left();
    Serial.println("电机启动 ： 左转");
    request->send(200, "text/plain", "turn_left");
  });

  // 后退处理
  server.on("/motor/back", HTTP_GET, [](AsyncWebServerRequest *request){
  backward();
  Serial.println("电机启动 ： 后退");
  request->send(200, "text/plain", "backWard");
  });

  // mode1
  server.on("/motor/mode1", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "mode1");
  });

  // mode1
  server.on("/motor/mode2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "mode2");
  });

  // mode2
  server.on("/motor/mode3", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "mode3");
  });

  server.begin(); // 启动服务器
}

void loop() {
  // 空循环
}
