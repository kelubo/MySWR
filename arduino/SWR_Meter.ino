// SWR驻波表 Arduino UNO 版本
// 作者：Trae AI
// 日期：2026-01-07

#include <LiquidCrystal.h>

// LCD 引脚配置
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// 输入引脚配置
const int forwardPowerPin = A0;  // 正向功率传感器输入
const int reflectedPowerPin = A1; // 反射功率传感器输入
const int buttonPin = 6;         // 功能按钮

// 变量定义
float forwardPower = 100.0;  // 正向功率 (W)
float reflectedPower = 0.0;   // 反射功率 (W)
float swrValue = 1.0;         // SWR 值
float reflectionCoeff = 0.0;  // 反射系数
float powerLoss = 0.0;        // 功率损失 (%)

int buttonState = 0;          // 按钮状态
int lastButtonState = 0;      // 上一次按钮状态
unsigned long lastDebounceTime = 0;  // 防抖动时间
unsigned long debounceDelay = 50;    // 防抖动延迟

int displayMode = 0;          // 显示模式 (0: SWR, 1: 功率, 2: 反射系数, 3: 功率损失)

void setup() {
  // 初始化 LCD
  lcd.begin(16, 2);
  
  // 设置输入引脚
  pinMode(buttonPin, INPUT_PULLUP);
  
  // 显示欢迎信息
  lcd.print("SWR Meter Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  // 读取按钮状态
  readButton();
  
  // 读取模拟输入值
  readPowerValues();
  
  // 计算 SWR 和相关参数
  calculateValues();
  
  // 更新 LCD 显示
  updateDisplay();
  
  // 延迟 100ms
  delay(100);
}

// 读取按钮状态（防抖动）
void readButton() {
  int reading = digitalRead(buttonPin);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW) {
        displayMode = (displayMode + 1) % 4;
        delay(200); // 额外延迟，避免多次触发
      }
    }
  }
  
  lastButtonState = reading;
}

// 读取功率值（模拟传感器）
void readPowerValues() {
  // 从模拟引脚读取值 (0-1023)
  int forwardReading = analogRead(forwardPowerPin);
  int reflectedReading = analogRead(reflectedPowerPin);
  
  // 转换为功率值 (假设传感器范围 0-1000W)
  forwardPower = map(forwardReading, 0, 1023, 0, 1000) / 10.0;
  reflectedPower = map(reflectedReading, 0, 1023, 0, 1000) / 10.0;
  
  // 确保反射功率不超过正向功率
  if (reflectedPower > forwardPower) {
    reflectedPower = forwardPower;
  }
}

// 计算 SWR 和相关参数
void calculateValues() {
  if (forwardPower == 0) {
    swrValue = INFINITY;
    reflectionCoeff = 0.0;
    powerLoss = 0.0;
  } else {
    reflectionCoeff = sqrt(reflectedPower / forwardPower);
    swrValue = (1 + reflectionCoeff) / (1 - reflectionCoeff);
    powerLoss = (reflectedPower / forwardPower) * 100;
  }
}

// 更新 LCD 显示
void updateDisplay() {
  lcd.clear();
  
  switch (displayMode) {
    case 0:  // 显示 SWR 值和状态
      lcd.setCursor(0, 0);
      lcd.print("SWR: ");
      
      if (swrValue == INFINITY) {
        lcd.print("----");
      } else {
        lcd.print(swrValue, 1);
      }
      
      lcd.setCursor(0, 1);
      lcd.print(getSWRStatus());
      break;
      
    case 1:  // 显示功率值
      lcd.setCursor(0, 0);
      lcd.print("FWD: ");
      lcd.print(forwardPower, 1);
      lcd.print("W");
      
      lcd.setCursor(0, 1);
      lcd.print("REF: ");
      lcd.print(reflectedPower, 1);
      lcd.print("W");
      break;
      
    case 2:  // 显示反射系数
      lcd.setCursor(0, 0);
      lcd.print("Refl Coeff:");
      
      lcd.setCursor(0, 1);
      lcd.print(reflectionCoeff, 2);
      break;
      
    case 3:  // 显示功率损失
      lcd.setCursor(0, 0);
      lcd.print("Power Loss:");
      
      lcd.setCursor(0, 1);
      lcd.print(powerLoss, 1);
      lcd.print("%");
      break;
  }
}

// 获取 SWR 状态文本
String getSWRStatus() {
  if (swrValue == INFINITY) {
    return "Error"; // 错误状态
  } else if (swrValue <= 1.5) {
    return "Good (1.0-1.5)"; // 良好
  } else if (swrValue <= 2.0) {
    return "Fair (1.5-2.0)";  // 一般
  } else if (swrValue <= 3.0) {
    return "Poor (2.0-3.0)";  // 较差
  } else {
    return "Bad (>3.0)";      // 严重
  }
}
