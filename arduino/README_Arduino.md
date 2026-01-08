# SWR驻波表 Arduino UNO 版本

这是一个基于Arduino UNO的SWR驻波表项目，可以测量和显示射频设备的驻波比(SWR)、正向功率、反射功率、反射系数和功率损失等参数。

## 硬件需求

### 核心组件
- Arduino UNO 开发板
- 16x2 LCD 显示屏 (带I2C接口或并行接口)
- 两个模拟功率传感器 (用于测量正向和反射功率)
- 功能按钮
- 面包板和连接线
- 5V 电源供应

### 可选组件
- 状态指示灯 (LED)
- 蜂鸣器 (用于警告)
- 电位器 (用于LCD对比度调节)

## 电路连接图

### LCD 显示屏连接 (并行接口)
- LCD RS → Arduino 12
- LCD EN → Arduino 11
- LCD D4 → Arduino 5
- LCD D5 → Arduino 4
- LCD D6 → Arduino 3
- LCD D7 → Arduino 2
- LCD VSS → GND
- LCD VDD → 5V
- LCD V0 → 电位器 (用于对比度调节)
- LCD A → 5V (通过限流电阻)
- LCD K → GND

### 功率传感器连接
- 正向功率传感器输出 → Arduino A0
- 反射功率传感器输出 → Arduino A1

### 按钮连接
- 功能按钮一端 → Arduino 6
- 功能按钮另一端 → GND

### 电源连接
- Arduino VIN → 5V 电源正极
- Arduino GND → 电源负极

## 软件安装和配置

### 安装Arduino IDE
1. 从[Arduino官方网站](https://www.arduino.cc/en/software)下载并安装Arduino IDE
2. 启动Arduino IDE并安装Arduino UNO的板支持包

### 安装依赖库
1. 打开Arduino IDE
2. 点击「项目」→「加载库」→「管理库...」
3. 搜索并安装「LiquidCrystal」库

## 上传和运行

1. 将Arduino UNO通过USB线连接到电脑
2. 在Arduino IDE中打开`SWR_Meter.ino`文件
3. 选择正确的开发板（Arduino UNO）和端口
4. 点击「上传」按钮将代码烧录到Arduino UNO
5. 等待上传完成，LCD显示屏将显示欢迎信息
6. 连接功率传感器和电源，即可开始测量

## 使用说明

### 基本操作
- **显示模式切换**：按下功能按钮可以在不同的显示模式之间切换
  - 模式0：显示SWR值和状态
  - 模式1：显示正向功率和反射功率
  - 模式2：显示反射系数
  - 模式3：显示功率损失百分比

### SWR状态说明
- **Good (1.0-1.5)**：良好状态，设备工作正常
- **Fair (1.5-2.0)**：一般状态，可以正常工作但建议优化
- **Poor (2.0-3.0)**：较差状态，可能会影响设备性能
- **Bad (>3.0)**：严重状态，可能会损坏设备，需要立即修复

## 功能特性

- ✅ 实时测量和显示SWR值
- ✅ 测量正向功率和反射功率
- ✅ 计算并显示反射系数
- ✅ 计算并显示功率损失百分比
- ✅ 多模式显示切换
- ✅ LCD显示屏友好界面
- ✅ 防抖动按钮处理
- ✅ 自动保护逻辑（防止反射功率超过正向功率）

## 注意事项

1. **传感器校准**：在使用前，请根据实际传感器的特性校准功率测量值
2. **电源供应**：确保使用稳定的5V电源，避免电压波动影响测量精度
3. **RF干扰**：尽量远离强射频干扰源，以提高测量准确性
4. **传感器保护**：避免超过传感器的最大测量范围，防止损坏传感器
5. **安全操作**：在测量高功率设备时，请遵循相关安全规定，避免触电或烫伤

## 代码修改说明

### 校准功率传感器
如果需要校准功率传感器，请修改`readPowerValues()`函数中的转换逻辑：

```cpp
void readPowerValues() {
  // 从模拟引脚读取值 (0-1023)
  int forwardReading = analogRead(forwardPowerPin);
  int reflectedReading = analogRead(reflectedPowerPin);
  
  // 转换为功率值 (根据实际传感器特性调整)
  forwardPower = map(forwardReading, 0, 1023, 0, 1000) / 10.0;
  reflectedPower = map(reflectedReading, 0, 1023, 0, 1000) / 10.0;
  
  // 确保反射功率不超过正向功率
  if (reflectedPower > forwardPower) {
    reflectedPower = forwardPower;
  }
}
```

### 修改LCD引脚配置
如果需要修改LCD引脚配置，请修改以下代码：

```cpp
// LCD 引脚配置
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
```

## 故障排除

### LCD不显示
1. 检查LCD引脚连接是否正确
2. 检查对比度电位器是否调节合适
3. 检查电源连接是否正常

### 功率测量不准确
1. 校准功率传感器
2. 检查传感器连接是否稳定
3. 避免RF干扰

### 按钮不工作
1. 检查按钮连接是否正确
2. 检查防抖动延迟设置是否合适

## 扩展功能

1. **添加I2C接口**：使用I2C LCD显示屏可以减少引脚使用
2. **添加存储功能**：使用EEPROM存储历史数据
3. **添加串行通信**：通过串口将数据发送到电脑
4. **添加无线通信**：使用蓝牙或WiFi模块远程监控
5. **添加报警功能**：当SWR超过阈值时发出警报

## 许可证

MIT License

## 联系方式

如有问题或建议，请通过以下方式联系：
- Email: contact@traeai.com
- GitHub: https://github.com/traeai

---

**注意：这是一个开源项目，仅供学习和参考。在实际应用中，请确保符合相关安全规定和标准。**
