# SWR驻波表项目

这是一个基于Arduino、Raspberry Pi和Web技术的SWR驻波表项目，可以实时测量和显示射频设备的驻波比(SWR)、正向功率、反射功率、反射系数和功率损失等参数。项目支持Arduino和Raspberry Pi两种硬件平台，并提供统一的Web访问界面。

## 项目结构

```
SWR_Meter/
├── README.md                  # 项目主文档
├── arduino/                   # Arduino相关文件目录
│   ├── README_Arduino.md      # Arduino硬件版本详细说明
│   ├── README_Remote.md       # Arduino远程控制功能详细说明
│   ├── SWR_Meter.ino          # Arduino UNO版本主程序
│   └── SWR_Meter_Ethernet.ino # Arduino远程控制版本主程序
├── raspberrypi/               # Raspberry Pi相关文件目录
│   ├── README_RaspberryPi.md  # Raspberry Pi版本详细说明
│   ├── swr_meter_raspberry_pi.py  # Raspberry Pi版本主程序
│   └── raspberry_pi_config.json   # Raspberry Pi配置文件
├── index.html                 # 前端界面主文件
├── script.js                  # 前端JavaScript逻辑
├── styles.css                 # 前端样式文件
├── package.json               # 项目依赖配置
└── test_server.js             # 本地测试服务器
```

## 功能特性

### 基础功能
- ✅ 实时测量和显示SWR值
- ✅ 测量正向功率和反射功率
- ✅ 计算并显示反射系数
- ✅ 计算并显示功率损失百分比
- ✅ 多模式显示切换
- ✅ 友好的数字表头界面

### 扩展功能
- ✅ 远程Web访问功能（Arduino需要Ethernet Shield）
- ✅ 基于颜色的状态指示
- ✅ 用户认证机制
- ✅ 报警设置功能
- ✅ 远程控制功能
- ✅ 历史数据查看（基础框架已实现）
- ✅ 支持Raspberry Pi平台
- ✅ 可配置的校准参数
- ✅ 轻量级实现，资源占用低

## 硬件版本（Arduino UNO）

这是一个基于Arduino UNO的SWR驻波表，可以直接通过LCD显示屏查看测量结果。

### 主要硬件组件
- Arduino UNO 开发板
- 16x2 LCD 显示屏
- 两个模拟功率传感器
- 功能按钮
- 电源供应

### 详细说明
请参考 [README_Arduino.md](arduino/README_Arduino.md) 文件获取完整的硬件连接、安装和使用说明。

## 远程控制版本

通过Arduino UNO + Ethernet Shield实现远程Web访问功能，可以在浏览器中实时查看SWR驻波表数据。

### 主要硬件组件
- Arduino UNO 开发板
- Ethernet Shield W5100
- 16x2 LCD 显示屏
- 功率传感器
- 功能按钮

### 详细说明
请参考 [README_Remote.md](arduino/README_Remote.md) 文件获取完整的硬件连接、网络配置和远程访问说明。

## Raspberry Pi版本

这是SWR驻波表项目的Raspberry Pi版本，用于在树莓派上运行SWR驻波表测量功能，并提供Web访问界面。

### 主要特性
- ✅ 实时测量正向功率和反射功率
- ✅ 计算并显示SWR值、反射系数和功率损失
- ✅ 基于Web的用户界面
- ✅ 支持远程访问和监控
- ✅ 可配置的校准参数
- ✅ 轻量级实现，资源占用低

### 硬件需求
- Raspberry Pi (推荐3代或更高版本)
- 功率传感器模块
- ADC模块 (如MCP3008)
- 杜邦线和面包板

### 详细说明
请参考 [README_RaspberryPi.md](raspberrypi/README_RaspberryPi.md) 文件获取完整的硬件连接、安装和使用说明。

## 前端界面使用

前端界面提供了一个现代化的Web界面，可以显示实时的SWR驻波表数据，并提供管理和控制功能。

### 主要功能
- **数字表头**：实时显示正向功率、SWR值和反向功率
- **结果显示**：显示详细的SWR值、反射系数和功率损失
- **状态指示器**：根据SWR值显示不同的状态（良好、一般、较差、严重）
- **动态颜色**：SWR区域的边框和数值会根据SWR值自动改变颜色
- **功能入口**：提供管理界面和历史数据的进入入口

## 安装和运行

### 本地运行前端界面（用于测试）

1. 确保安装了Node.js
2. 安装依赖：
   ```bash
   npm install
   ```
3. 启动本地服务器：
   ```bash
   npm start
   ```
4. 在浏览器中访问：`http://localhost:3000`

### Arduino版本运行

请参考 [README_Arduino.md](arduino/README_Arduino.md) 文件获取Arduino版本的安装和运行说明。

### Raspberry Pi版本运行

1. 确保树莓派已安装最新的Raspbian系统
2. 安装必要的依赖：
   ```bash
   sudo apt-get update
   sudo apt-get install -y python3 python3-pip python3-rpi.gpio python3-smbus
   ```
3. 克隆项目：
   ```bash
   git clone https://github.com/yourusername/SWR_Meter.git
   cd SWR_Meter
   ```
4. 运行程序：
   ```bash
   python3 raspberrypi/swr_meter_raspberry_pi.py
   ```

5. 在浏览器中访问：`http://<raspberry_pi_ip>:8080`

详细说明请参考 [README_RaspberryPi.md](raspberrypi/README_RaspberryPi.md) 文件。

### 远程访问

- **Arduino版本**：请参考 [README_Remote.md](arduino/README_Remote.md) 文件获取远程访问说明。
- **Raspberry Pi版本**：默认开放8080端口，可直接通过网络访问。

## 使用说明

### 基础使用
1. **查看实时数据**：打开界面后，数字表头会实时显示正向功率、SWR值和反向功率
2. **了解设备状态**：根据SWR值和状态指示器了解设备的工作状态
3. **查看详细信息**：在结果显示区域查看详细的SWR值、反射系数和功率损失

### 高级功能

#### 管理界面
1. 点击首页的"管理界面"按钮
2. 如果未登录，会弹出登录界面（默认用户名：admin，密码：password）
3. 登录后，会显示报警设置和远程控制区域
4. **报警设置**：可以设置SWR报警阈值并启用/禁用报警功能
5. **远程控制**：可以执行重置设备、校准和保存设置等操作

#### 历史数据
1. 点击首页的"历史数据"按钮
2. 如果未登录，会弹出登录界面
3. 登录后，会显示历史数据区域
4. 可以选择不同的时间范围查看历史数据

### SWR状态说明
- **良好 (1.0-1.5)**：设备工作正常
- **一般 (1.5-2.0)**：可以正常工作但建议优化
- **较差 (2.0-3.0)**：可能会影响设备性能
- **严重 (>3.0)**：可能会损坏设备，需要立即修复

## 注意事项

1. **传感器校准**：在使用前，请根据实际传感器的特性校准功率测量值
2. **电源供应**：确保使用稳定的电源，避免电压波动影响测量精度
3. **RF干扰**：尽量远离强射频干扰源，以提高测量准确性
4. **传感器保护**：避免超过传感器的最大测量范围，防止损坏传感器
5. **安全操作**：在测量高功率设备时，请遵循相关安全规定，避免触电或烫伤
6. **用户认证**：远程访问和管理功能需要用户认证，请勿在生产环境中使用默认密码

## 许可证

MIT License

## 联系方式

如有问题或建议，请通过以下方式联系：
- Email: contact@traeai.com
- GitHub: https://github.com/traeai

---

**注意：这是一个开源项目，仅供学习和参考。在实际应用中，请确保符合相关安全规定和标准。**