// SWR驻波表 Arduino UNO + Ethernet Shield 版本
// 支持远程Web访问
// 作者：Trae AI
// 日期：2026-01-07

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

// LCD 引脚配置
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// 输入引脚配置
const int forwardPowerPin = A0;  // 正向功率传感器输入
const int reflectedPowerPin = A1; // 反射功率传感器输入
const int buttonPin = 6;         // 功能按钮

// 以太网配置
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);   // Arduino IP地址
EthernetServer server(80);         // Web服务器端口

// 认证配置
const char* authUsername = "admin";  // 认证用户名
const char* authPassword = "password";  // 认证密码

// 报警配置
float alarmThreshold = 2.0;    // SWR报警阈值
bool alarmEnabled = true;      // 报警是否启用
bool alarmTriggered = false;   // 报警是否触发

// 历史数据配置
const int HISTORY_SIZE = 60;   // 历史数据点数量 (Arduino UNO内存有限，最多存储60个点)
struct HistoryPoint {
  float forwardPower;
  float reflectedPower;
  float swrValue;
  unsigned long timestamp;     // 时间戳 (毫秒)
};
HistoryPoint history[HISTORY_SIZE];  // 历史数据数组
int historyIndex = 0;          // 当前历史数据索引

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

int displayMode = 0;          // 显示模式
unsigned long lastHistoryUpdate = 0;  // 上一次历史数据更新时间
unsigned long historyUpdateInterval = 1000;  // 历史数据更新间隔 (1秒)

void setup() {
  // 初始化 LCD
  lcd.begin(16, 2);
  
  // 设置输入引脚
  pinMode(buttonPin, INPUT_PULLUP);
  
  // 初始化以太网
  Ethernet.begin(mac, ip);
  server.begin();
  
  // 初始化历史数据数组
  for (int i = 0; i < HISTORY_SIZE; i++) {
    history[i].forwardPower = 0.0;
    history[i].reflectedPower = 0.0;
    history[i].swrValue = 1.0;
    history[i].timestamp = millis();
  }
  
  // 显示信息
  lcd.print("SWR Meter Ready");
  lcd.setCursor(0, 1);
  lcd.print("IP: 192.168.1.177");
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
  
  // 更新历史数据
  updateHistoryData();
  
  // 检测报警
  checkAlarm();
  
  // 更新 LCD 显示
  updateDisplay();
  
  // 处理 Web 请求
  handleWebRequests();
  
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
        delay(200);
      }
    }
  }
  
  lastButtonState = reading;
}

// 读取功率值
void readPowerValues() {
  int forwardReading = analogRead(forwardPowerPin);
  int reflectedReading = analogRead(reflectedPowerPin);
  
  // 转换为功率值 (可根据实际传感器校准)
  forwardPower = map(forwardReading, 0, 1023, 0, 1000) / 10.0;
  reflectedPower = map(reflectedReading, 0, 1023, 0, 1000) / 10.0;
  
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

// 更新历史数据
void updateHistoryData() {
  unsigned long currentTime = millis();
  
  // 每秒更新一次历史数据
  if (currentTime - lastHistoryUpdate >= historyUpdateInterval) {
    lastHistoryUpdate = currentTime;
    
    // 保存当前数据到历史数组
    history[historyIndex].forwardPower = forwardPower;
    history[historyIndex].reflectedPower = reflectedPower;
    history[historyIndex].swrValue = swrValue;
    history[historyIndex].timestamp = currentTime;
    
    // 更新历史数据索引（环形缓冲区）
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
  }
}

// 检测报警
void checkAlarm() {
  if (alarmEnabled) {
    if (swrValue != INFINITY && swrValue > alarmThreshold) {
      alarmTriggered = true;
    } else {
      alarmTriggered = false;
    }
  } else {
    alarmTriggered = false;
  }
}

// 处理认证
bool handleAuthentication(EthernetClient client) {
  // 简单的基本认证（实际应用中应该使用更安全的认证方式）
  client.println("HTTP/1.1 401 Unauthorized");
  client.println("WWW-Authenticate: Basic realm=\"SWR Meter\"");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("Authentication required");
  client.stop();
  return false;
}

// 解析JSON命令
void parseJSONCommand(String command, EthernetClient client) {
  // 简单的JSON命令解析（实际应用中应该使用JSON库）
  if (command.indexOf("reset") > -1) {
    // 重置设备
    forwardPower = 0.0;
    reflectedPower = 0.0;
    swrValue = 1.0;
    reflectionCoeff = 0.0;
    powerLoss = 0.0;
    sendJSONResponse(client, true, "Device reset");
  } else if (command.indexOf("calibrate") > -1) {
    // 校准设备
    // 这里添加校准逻辑
    sendJSONResponse(client, true, "Device calibrated");
  } else if (command.indexOf("save_settings") > -1) {
    // 保存设置
    // 这里添加保存设置逻辑
    sendJSONResponse(client, true, "Settings saved");
  } else {
    sendJSONResponse(client, false, "Unknown command");
  }
}

// 发送JSON响应
void sendJSONResponse(EthernetClient client, bool success, String message) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.print("{\"success\": ");
  client.print(success ? "true" : "false");
  client.print(", \"message\": \"");
  client.print(message);
  client.print("\"}");
  client.println();
}

// 发送历史数据
void sendHistoryData(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  
  client.print("{");
  client.print("\"history\": [");
  
  for (int i = 0; i < HISTORY_SIZE; i++) {
    int index = (historyIndex + i) % HISTORY_SIZE;
    
    client.print("{");
    client.print("\"timestamp\": ");
    client.print(history[index].timestamp);
    client.print(", \"forwardPower\": ");
    client.print(history[index].forwardPower, 1);
    client.print(", \"reflectedPower\": ");
    client.print(history[index].reflectedPower, 1);
    client.print(", \"swrValue\": ");
    
    if (history[index].swrValue == INFINITY) {
      client.print("\"Infinity\"");
    } else {
      client.print(history[index].swrValue, 1);
    }
    
    client.print("}");
    
    if (i < HISTORY_SIZE - 1) {
      client.print(",");
    }
  }
  
  client.print("]}");
  client.println();
}

// 发送报警数据
void sendAlarmData(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  
  client.print("{");
  client.print("\"threshold\": ");
  client.print(alarmThreshold, 1);
  client.print(", \"enabled\": ");
  client.print(alarmEnabled ? "true" : "false");
  client.print(", \"triggered\": ");
  client.print(alarmTriggered ? "true" : "false");
  client.print("}");
  client.println();
}

// 更新 LCD 显示
void updateDisplay() {
  lcd.clear();
  
  switch (displayMode) {
    case 0:
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
      
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("FWD: ");
      lcd.print(forwardPower, 1);
      lcd.print("W");
      lcd.setCursor(0, 1);
      lcd.print("REF: ");
      lcd.print(reflectedPower, 1);
      lcd.print("W");
      break;
      
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Refl Coeff:");
      lcd.setCursor(0, 1);
      lcd.print(reflectionCoeff, 2);
      break;
      
    case 3:
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

// 处理 Web 请求
void handleWebRequests() {
  EthernetClient client = server.available();
  
  if (client) {
    String currentLine = "";
    bool isGetRequest = false;
    bool isDataRequest = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // 发送 HTTP 头
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            if (isDataRequest) {
              // 发送 JSON 数据
              sendJSONData(client);
            } else {
              // 发送 HTML 页面
              sendHTMLPage(client);
            }
            break;
          } else {
            // 解析请求
            if (currentLine.startsWith("GET /data") || currentLine.startsWith("GET /api/swr")) {
              isDataRequest = true;
            }
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    client.stop();
  }
}

// 发送 HTML 页面
void sendHTMLPage(EthernetClient client) {
  client.print(F("<!DOCTYPE html>
"));
  client.print(F("<html lang='zh-CN'>
"));
  client.print(F("<head>
"));
  client.print(F("    <meta charset='UTF-8'>
"));
  client.print(F("    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
"));
  client.print(F("    <title>SWR驻波表</title>
"));
  client.print(F("    <style>
"));
  client.print(F("        * { margin: 0; padding: 0; box-sizing: border-box; }
"));
  client.print(F("        body { font-family: Arial, sans-serif; background-color: #f0f0f0; color: #333; 
"));
  client.print(F("               display: flex; justify-content: center; align-items: center; min-height: 100vh; 
"));
  client.print(F("               padding: 20px; }
"));
  client.print(F("        .container { background-color: #fff; border-radius: 15px; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2); 
"));
  client.print(F("                    padding: 30px 20px; max-width: 500px; width: 100%; margin: 0 auto; }
"));
  client.print(F("        h1 { text-align: center; margin-bottom: 30px; color: #2c3e50; font-size: 28px; }
"));
  client.print(F("        .meter-container { position: relative; margin-bottom: 40px; 
"));
  client.print(F("                          display: flex; justify-content: center; align-items: center; }
"));
  client.print(F("        .digital-meter { display: flex; justify-content: center; align-items: center; gap: 15px; 
"));
  client.print(F("                          padding: 30px; background-color: #f9f1c7; border-radius: 15px; 
"));
  client.print(F("                          box-shadow: 0 5px 15px rgba(0, 0, 0, 0.4); border: 3px solid #333; 
"));
  client.print(F("                          background-image: linear-gradient(to bottom, #f9f1c7 0%, #fffac8 100%); 
"));
  client.print(F("                          width: 420px; margin: 0 auto; }
"));
  client.print(F("        .meter-section { text-align: center; padding: 15px 10px; border-radius: 10px; 
"));
  client.print(F("                         background-color: rgba(255, 255, 255, 0.8); 
"));
  client.print(F("                         box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.1); border: 2px solid #ddd; 
"));
  client.print(F("                         width: 130px; height: 140px; box-sizing: border-box; 
"));
  client.print(F("                         display: flex; flex-direction: column; justify-content: center; 
"));
  client.print(F("                         align-items: center; }
"));
  client.print(F("        .forward-section { border-color: #27ae60; }
"));
  client.print(F("        .swr-section { border-color: #e74c3c; z-index: 1; }
"));
  client.print(F("        .reflected-section { border-color: #e74c3c; }
"));
  client.print(F("        .meter-label { font-size: 14px; font-weight: bold; color: #333; margin-bottom: 5px; 
"));
  client.print(F("                       text-transform: uppercase; letter-spacing: 1px; line-height: 1; }
"));
  client.print(F("        .meter-value { font-size: 26px; font-weight: bold; color: #2c3e50; margin-bottom: 5px; 
"));
  client.print(F("                       font-family: 'Courier New', monospace; 
"));
  client.print(F("                       text-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); line-height: 1; }
"));
  client.print(F("        .forward-section .meter-value { color: #27ae60; }
"));
  client.print(F("        .swr-section .meter-value { color: #e74c3c; font-size: 30px; }
"));
  client.print(F("        .reflected-section .meter-value { color: #e74c3c; }
"));
  client.print(F("        .meter-unit { font-size: 18px; font-weight: bold; color: #666; text-transform: uppercase; 
"));
  client.print(F("                      line-height: 1; }
"));
  client.print(F("        .status-indicator { display: flex; flex-direction: column; gap: 10px; }
"));
  client.print(F("        .status-item { display: flex; align-items: center; padding: 10px; border-radius: 5px; 
"));
  client.print(F("                       transition: all 0.3s ease; opacity: 0.5; margin-bottom: 10px; }
"));
  client.print(F("        .status-item:last-child { margin-bottom: 0; }
"));
  client.print(F("        .status-item.active { opacity: 1; font-weight: bold; }
"));
  client.print(F("        .status-dot { width: 12px; height: 12px; border-radius: 50%; margin-right: 10px; }
"));
  client.print(F("        #statusGood .status-dot { background-color: #27ae60; }
"));
  client.print(F("        #statusFair .status-dot { background-color: #f39c12; }
"));
  client.print(F("        #statusPoor .status-dot { background-color: #e67e22; }
"));
  client.print(F("        #statusBad .status-dot { background-color: #e74c3c; }
"));
  client.print(F("        #statusGood.active { background-color: rgba(39, 174, 96, 0.1); border-left: 4px solid #27ae60; }
"));
  client.print(F("        #statusFair.active { background-color: rgba(243, 156, 18, 0.1); border-left: 4px solid #f39c12; }
"));
  client.print(F("        #statusPoor.active { background-color: rgba(230, 126, 34, 0.1); border-left: 4px solid #e67e22; }
"));
  client.print(F("        #statusBad.active { background-color: rgba(231, 76, 60, 0.1); border-left: 4px solid #e74c3c; }
"));
  client.print(F("        .result-group { display: flex; justify-content: space-around; gap: 15px; margin-top: 25px; }
"));
  client.print(F("        .result-item { text-align: center; padding: 15px; background-color: #f8f9fa; border-radius: 8px; 
"));
  client.print(F("                       border: 1px solid #e9ecef; flex: 1; }
"));
  client.print(F("        .result-item .label { display: block; font-size: 14px; color: #6c757d; margin-bottom: 5px; }
"));
  client.print(F("        .result-item .value { display: block; font-size: 20px; font-weight: bold; color: #2c3e50; }
"));
  client.print(F("    </style>
"));
  client.print(F("</head>
"));
  client.print(F("<body>
"));
  client.print(F("    <div class='container'>
"));
  client.print(F("        <h1>SWR驻波表 (远程监控)</h1>
"));
  client.print(F("        
"));
  client.print(F("        <div class='meter-container'>
"));
  client.print(F("            <div class='digital-meter'>
"));
  client.print(F("                <div class='meter-section forward-section'>
"));
  client.print(F("                    <div class='meter-label'>正向功率</div>
"));
  client.print(F("                    <div class='meter-value' id='forwardValue'>0.0</div>
"));
  client.print(F("                    <div class='meter-unit'>W</div>
"));
  client.print(F("                </div>
"));
  client.print(F("                <div class='meter-section swr-section'>
"));
  client.print(F("                    <div class='meter-label'>SWR</div>
"));
  client.print(F("                    <div class='meter-value' id='swrValue'>1.0</div>
"));
  client.print(F("                    <div class='meter-unit'></div>
"));
  client.print(F("                </div>
"));
  client.print(F("                <div class='meter-section reflected-section'>
"));
  client.print(F("                    <div class='meter-label'>反向功率</div>
"));
  client.print(F("                    <div class='meter-value' id='reflectedValue'>0.0</div>
"));
  client.print(F("                    <div class='meter-unit'>W</div>
"));
  client.print(F("                </div>
"));
  client.print(F("            </div>
"));
  client.print(F("        </div>
"));
  client.print(F("        
"));
  client.print(F("        <div class='result-group'>
"));
  client.print(F("            <div class='result-item'>
"));
  client.print(F("                <span class='label'>SWR:</span>
"));
  client.print(F("                <span class='value' id='calculatedSWR'>1.0</span>
"));
  client.print(F("            </div>
"));
  client.print(F("            <div class='result-item'>
"));
  client.print(F("                <span class='label'>反射系数:</span>
"));
  client.print(F("                <span class='value' id='reflectionCoeff'>0.0</span>
"));
  client.print(F("            </div>
"));
  client.print(F("            <div class='result-item'>
"));
  client.print(F("                <span class='label'>功率损失:</span>
"));
  client.print(F("                <span class='value' id='powerLoss'>0.0%</span>
"));
  client.print(F("            </div>
"));
  client.print(F("        </div>
"));
  client.print(F("        
"));
  client.print(F("        <div class='status-indicator'>
"));
  client.print(F("            <div class='status-item' id='statusGood' class='active'>
"));
  client.print(F("                <span class='status-dot'></span>
"));
  client.print(F("                <span>良好 (1.0-1.5)</span>
"));
  client.print(F("            </div>
"));
  client.print(F("            <div class='status-item' id='statusFair'>
"));
  client.print(F("                <span class='status-dot'></span>
"));
  client.print(F("                <span>一般 (1.5-2.0)</span>
"));
  client.print(F("            </div>
"));
  client.print(F("            <div class='status-item' id='statusPoor'>
"));
  client.print(F("                <span class='status-dot'></span>
"));
  client.print(F("                <span>较差 (2.0-3.0)</span>
"));
  client.print(F("            </div>
"));
  client.print(F("            <div class='status-item' id='statusBad'>
"));
  client.print(F("                <span class='status-dot'></span>
"));
  client.print(F("                <span>严重 (>3.0)</span>
"));
  client.print(F("            </div>
"));
  client.print(F("        </div>
"));
  client.print(F("    </div>
"));
  client.print(F("    
"));
  client.print(F("    <script>
"));
  client.print(F("        // 更新显示
"));
  client.print(F("        function updateDisplay(data) {
"));
  client.print(F("            // 更新数字表头
"));
  client.print(F("            document.getElementById('forwardValue').textContent = data.forwardPower.toFixed(1);
"));
  client.print(F("            document.getElementById('reflectedValue').textContent = data.reflectedPower.toFixed(1);
"));
  client.print(F("            
"));
  client.print(F("            // 更新SWR值
"));
  client.print(F("            if (data.swrValue === Infinity || isNaN(data.swrValue) || data.swrValue === \"Infinity\") {
"));
  client.print(F("                document.getElementById('swrValue').textContent = '∞';
"));
  client.print(F("                document.getElementById('calculatedSWR').textContent = '∞';
"));
  client.print(F("            } else {
"));
  client.print(F("                const swrFixed = parseFloat(data.swrValue).toFixed(1);
"));
  client.print(F("                document.getElementById('swrValue').textContent = swrFixed;
"));
  client.print(F("                document.getElementById('calculatedSWR').textContent = swrFixed;
"));
  client.print(F("            }
"));
  client.print(F("            
"));
  client.print(F("            // 更新反射系数
"));
  client.print(F("            document.getElementById('reflectionCoeff').textContent = data.reflectionCoeff.toFixed(2);
"));
  client.print(F("            
"));
  client.print(F("            // 更新功率损失
"));
  client.print(F("            document.getElementById('powerLoss').textContent = data.powerLoss.toFixed(1) + '%';
"));
  client.print(F("            
"));
  client.print(F("            // 更新状态指示器
"));
  client.print(F("            const swrForStatus = (data.swrValue === \"Infinity\") ? Infinity : parseFloat(data.swrValue);
"));
  client.print(F("            updateStatus(swrForStatus);
"));
  client.print(F("        }
"));
  client.print(F("        
"));
  client.print(F("        // 更新状态指示器
"));
  client.print(F("        function updateStatus(swr) {
"));
  client.print(F("            // 重置所有状态
"));
  client.print(F("            document.getElementById('statusGood').classList.remove('active');
"));
  client.print(F("            document.getElementById('statusFair').classList.remove('active');
"));
  client.print(F("            document.getElementById('statusPoor').classList.remove('active');
"));
  client.print(F("            document.getElementById('statusBad').classList.remove('active');
"));
  client.print(F("            
"));
  client.print(F("            // 更新SWR边框和数值颜色
"));
  client.print(F("            const swrSection = document.querySelector('.swr-section');
"));
  client.print(F("            const swrValueElement = document.getElementById('swrValue');
"));
  client.print(F("            
"));
  client.print(F("            if (swr === Infinity || isNaN(swr)) {
"));
  client.print(F("                swrSection.style.borderColor = '#ddd';
"));
  client.print(F("                swrValueElement.style.color = '#2c3e50';
"));
  client.print(F("            } else if (swr <= 1.5) {
"));
  client.print(F("                document.getElementById('statusGood').classList.add('active');
"));
  client.print(F("                swrSection.style.borderColor = '#27ae60'; // 绿色
"));
  client.print(F("                swrValueElement.style.color = '#27ae60';
"));
  client.print(F("            } else if (swr <= 2) {
"));
  client.print(F("                document.getElementById('statusFair').classList.add('active');
"));
  client.print(F("                swrSection.style.borderColor = '#f39c12'; // 橙色
"));
  client.print(F("                swrValueElement.style.color = '#f39c12';
"));
  client.print(F("            } else if (swr <= 3) {
"));
  client.print(F("                document.getElementById('statusPoor').classList.add('active');
"));
  client.print(F("                swrSection.style.borderColor = '#e67e22'; // 深橙色
"));
  client.print(F("                swrValueElement.style.color = '#e67e22';
"));
  client.print(F("            } else {
"));
  client.print(F("                document.getElementById('statusBad').classList.add('active');
"));
  client.print(F("                swrSection.style.borderColor = '#e74c3c'; // 红色
"));
  client.print(F("                swrValueElement.style.color = '#e74c3c';
"));
  client.print(F("            }
"));
  client.print(F("        }
"));
  client.print(F("        
"));
  client.print(F("        // 轮询数据
"));
  client.print(F("        function fetchData() {
"));
  client.print(F("            fetch('/data')
"));
  client.print(F("                .then(response => response.json())
"));
  client.print(F("                .then(data => {
"));
  client.print(F("                    updateDisplay(data);
"));
  client.print(F("                }))
"));
  client.print(F("                .catch(error => {
"));
  client.print(F("                    console.error('获取数据失败:', error);
"));
  client.print(F("                });
"));
  client.print(F("        }
"));
  client.print(F("        
"));
  client.print(F("        // 初始化显示
"));
  client.print(F("        fetchData();
"));
  client.print(F("        
"));
  client.print(F("        // 设置定时轮询 (每1秒)
"));
  client.print(F("        setInterval(fetchData, 1000);
"));
  client.print(F("    </script>
"));
  client.print(F("</body>
"));
  client.print(F("</html>
"));
  client.println();
}

// 发送 JSON 数据
void sendJSONData(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  
  // 构建 JSON 字符串（与Raspberry Pi API格式兼容）
  client.print("{\n");
  client.print("  \"forward_power\": ");
  client.print(forwardPower, 1);
  client.print(",\n");
  client.print("  \"reverse_power\": ");
  client.print(reflectedPower, 1);
  client.print(",\n");
  client.print("  \"swr\": ");
  if (swrValue == INFINITY) {
    client.print("\"Infinity\"");
  } else {
    client.print(swrValue, 1);
  }
  client.print(",\n");
  client.print("  \"reflection_coefficient\": ");
  client.print(reflectionCoeff, 2);
  client.print(",\n");
  client.print("  \"power_loss\": ");
  client.print(powerLoss, 1);
  client.print(",\n");
  client.print("  \"timestamp\": ");
  client.print(millis());
  client.print("\n");
  client.print("}");
  client.println();
}
