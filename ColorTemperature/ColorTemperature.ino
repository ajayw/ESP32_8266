#include <Adafruit_NeoPixel.h>

// 定义LED相关参数
#define LED_PIN 14     // LED接在数字引脚14上
#define NUM_LEDS 3     // LED个数
#define BRIGHTNESS 64  // 设置亮度为64 (范围0-255)

// 创建一个NeoPixel对象
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

// 定义一个结构体来表示色温
typedef struct {
  const char* name;  // 色温名称
  uint32_t color;    // RGBW值，使用uint32_t表示颜色
} ColorTemperature;

// 色温数组，存储不同的色温及其对应的RGBW值
ColorTemperature temperatureArray[] = {
  { "Candle", strip.Color(255, 147, 41, 0) },                    //蜡烛
  { "Tungsten40W", strip.Color(255, 197, 143, 0) },              //40w钨丝灯
  { "Tungsten100W", strip.Color(255, 214, 170, 0) },             //100w钨丝灯
  { "Halogen", strip.Color(255, 241, 224, 0) },                  //卤素
  { "CarbonArc", strip.Color(255, 250, 244, 0) },                //碳弧
  { "HighNoonSun", strip.Color(255, 255, 251, 0) },              //正午太阳
  { "DirectSunlight", strip.Color(255, 255, 255, 0) },           //阳光直射
  { "OvercastSky", strip.Color(201, 226, 255, 0) },              //阴天
  { "ClearBlueSky", strip.Color(64, 156, 255, 0) },              //晴空万里
  { "WarmFluorescent", strip.Color(255, 244, 229, 0) },          //温暖的荧光
  { "StandardFluorescent", strip.Color(244, 255, 250, 0) },      //标准荧光
  { "CoolWhiteFluorescent", strip.Color(212, 235, 255, 0) },     //冷白荧光
  { "FullSpectrumFluorescent", strip.Color(255, 244, 242, 0) },  //全光谱荧光
  { "GrowLightFluorescent", strip.Color(255, 239, 247, 0) },     //生光荧光灯
  { "BlackLightFluorescent", strip.Color(167, 0, 255, 0) },      //黑光荧光灯
  { "MercuryVapor", strip.Color(216, 247, 255, 0) },             //汞蒸气
  { "SodiumVapor", strip.Color(255, 209, 178, 0) },              //钠蒸汽
  { "MetalHalide", strip.Color(242, 252, 255, 0) },              //金属卤素灯
  { "HighPressureSodium", strip.Color(255, 183, 76, 0) }         //高压钠
};

void setup() {
  // 初始化NeoPixel
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();  // 将所有LED初始化为关闭状态
}

void loop() {
  // 遍历色温数组，并设置每个LED的颜色
  for (int i = 0; i < sizeof(temperatureArray) / sizeof(ColorTemperature); ++i) {
    // 取出当前的色温
    uint32_t currentColor = temperatureArray[i].color;

    // 将所有LED设置为currentColor
    for (int j = 0; j < NUM_LEDS; ++j) {
      strip.setPixelColor(j, currentColor);
    }

    // 显示新设置的颜色
    strip.show();

    // 等待3秒
    delay(3000);
  }
}
