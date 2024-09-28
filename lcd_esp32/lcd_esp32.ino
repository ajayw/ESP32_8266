#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

// 替换为你的WiFi SSID和密码
const char* ssid = "xxxxxx";
const char* password = "xxxxxxxx";

// 创建一个LCD对象，地址为0x27，16列2行
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 创建Web服务器对象，使用默认的80端口
WebServer server(80);

void handleRoot() {
  // 定义一个简单的HTML页面
  String html = "<html><body><h1>ESP32 Text Sender</h1>";
  html += "<input type=\"text\" id=\"textInput\" maxlength=\"32\">";
  html += "<button onclick=\"sendText()\">Send</button>";
  html += "<p id=\"response\"></p>";
  html += "<script>";
  html += "function sendText() {";
  html += "  var text = document.getElementById('textInput').value;";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('POST', '/sendText', true);";
  html += "  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('response').innerText = 'Text Sent!';";
  html += "    }";
  html += "  };";
  html += "  xhr.send('text=' + encodeURIComponent(text));";
  html += "}";
  html += "</script></body></html>";
  
  // 发送HTML页面到浏览器
  server.send(200, "text/html", html);
}

void handleSendText() {
  // 获取POST请求中的文本数据
  if (server.hasArg("text")) {
    String text = server.arg("text");

    // 将文本数据显示在LCD1602上
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(text.substring(0, 16)); // 显示前16个字符
    if (text.length() > 16) {
      lcd.setCursor(0, 1);
      lcd.print(text.substring(16, 32)); // 显示接下来的16个字符
    }

    // 回复客户端
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error: No text provided!");
  }
}

void setup() {
  // 初始化串口用于调试
  Serial.begin(115200);

  // 初始化LCD
  lcd.init();
  lcd.backlight();

  // 连接到WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("正在连接WiFi...");
  }
  Serial.println("WiFi连接成功");
  Serial.println(WiFi.localIP());

  // 设置路由
  server.on("/", handleRoot);
  server.on("/sendText", HTTP_POST, handleSendText);

  // 启动服务器
  server.begin();
  Serial.println("服务器已启动");
}

void loop() {
  // 处理客户端请求
  server.handleClient();
}