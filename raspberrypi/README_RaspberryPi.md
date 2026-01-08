# SWR驻波表 - Raspberry Pi版本

## 项目概述
这是SWR驻波表项目的Raspberry Pi版本，用于在树莓派上运行SWR驻波表测量功能，并提供Web访问界面。

## 主要特性

- ✅ 实时测量正向功率和反射功率
- ✅ 计算并显示SWR值、反射系数和功率损失
- ✅ 基于Web的用户界面
- ✅ 支持远程访问和监控
- ✅ 可配置的校准参数
- ✅ 轻量级实现，资源占用低

## 硬件需求

### 基础组件
- Raspberry Pi (推荐3代或更高版本)
- 功率传感器模块 (如2x 5W或2x 10W定向耦合器 + 模拟输出传感器)
- ADC模块 (如MCP3008，用于读取模拟传感器信号)
- 杜邦线和面包板
- 电源适配器 (5V, 2.5A或更高)

### 可选组件
- 16x2 LCD显示屏 (用于本地显示)
- 按钮或旋钮 (用于本地控制)
- 外壳 (保护设备)

## 硬件连接

### ADC连接 (MCP3008示例)

| MCP3008 | Raspberry Pi GPIO |
|---------|-------------------|
| VDD     | 3.3V (Pin 17)     |
| VREF    | 3.3V (Pin 1)      |
| AGND    | GND (Pin 6)       |
| CLK     | GPIO 18 (Pin 12)  |
| DOUT    | GPIO 23 (Pin 16)  |
| DIN     | GPIO 24 (Pin 18)  |
| CS/SHDN | GPIO 25 (Pin 22)  |
| DGND    | GND (Pin 9)       |

### 传感器连接

- 正向功率传感器 → MCP3008 CH0
- 反射功率传感器 → MCP3008 CH1

## 软件安装

### 1. 准备Raspberry Pi

确保树莓派已安装最新的Raspbian系统，并已启用SSH和WiFi/以太网连接。

### 2. 安装必要的依赖

```bash
# 更新系统
 sudo apt-get update
 sudo apt-get upgrade -y

# 安装Python和必要的库
 sudo apt-get install -y python3 python3-pip python3-rpi.gpio python3-smbus

# 安装ADC模块库 (如果使用MCP3008)
 pip3 install adafruit-circuitpython-mcp3xxx
```

### 3. 下载项目文件

```bash
# 克隆项目仓库
 git clone https://github.com/yourusername/SWR_Meter.git
 cd SWR_Meter
```

### 4. 配置参数

编辑配置文件 `raspberry_pi_config.json` 调整参数：

```bash
 nano raspberry_pi_config.json
```

主要配置项：
- `calibration`: 校准系数
- `pins`: GPIO引脚配置
- `measurement`: 测量参数
- `web_server`: Web服务器配置

### 5. 运行程序

```bash
# 直接运行
 python3 swr_meter_raspberry_pi.py

# 或使用nohup后台运行
 nohup python3 swr_meter_raspberry_pi.py > swr.log 2>&1 &
```

## 访问界面

### 本地访问
打开浏览器，访问：
```
 http://localhost:8080
```

### 远程访问
在同一网络下，使用树莓派的IP地址访问：
```
 http://<raspberry_pi_ip>:8080
```

## API接口

### 获取SWR数据
```
 GET /api/swr
```

返回示例：
```json
{
  "forward_power": 5.2,
  "reverse_power": 0.1,
  "swr": 1.2,
  "reflection_coefficient": 0.09,
  "power_loss": 0.81,
  "timestamp": "2023-10-15 14:30:45"
}
```

### 获取配置信息
```
 GET /api/config
```

## 校准说明

1. 连接已知功率的信号源
2. 调整 `calibration.forward` 使正向功率显示准确
3. 调整 `calibration.reverse` 使反射功率显示准确
4. 保存配置

## 开机自启动

### 使用systemd设置自启动

1. 创建服务文件
```bash
 sudo nano /etc/systemd/system/swr_meter.service
```

2. 写入以下内容
```ini
[Unit]
Description=SWR驻波表服务
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/SWR_Meter/raspberrypi
ExecStart=/usr/bin/python3 /home/pi/SWR_Meter/raspberrypi/swr_meter_raspberry_pi.py
Restart=always

[Install]
WantedBy=multi-user.target
```

3. 启用并启动服务
```bash
 sudo systemctl daemon-reload
 sudo systemctl enable swr_meter.service
 sudo systemctl start swr_meter.service
```

4. 查看服务状态
```bash
 sudo systemctl status swr_meter.service
```

## 故障排除

### 常见问题

1. **Web界面无法访问**
   - 检查树莓派IP地址是否正确
   - 检查防火墙设置
   - 检查Web服务器是否正在运行

2. **测量值不准确**
   - 检查传感器连接
   - 重新校准传感器
   - 检查ADC模块连接

3. **程序无法启动**
   - 检查Python依赖是否安装完整
   - 检查GPIO引脚配置
   - 查看错误日志

### 日志查看

```bash
# 实时查看日志
 tail -f swr.log

# 查看系统日志
 sudo journalctl -u swr_meter.service
```

## 性能优化

1. **降低测量频率**：在配置文件中增加 `measurement.interval` 值
2. **关闭不必要的功能**：如数据库功能
3. **使用轻量级浏览器**：在低性能设备上使用Midori等轻量级浏览器

## 安全建议

1. **更改默认密码**：为树莓派用户设置强密码
2. **启用防火墙**：使用`ufw`或`iptables`限制访问
3. **使用HTTPS**：在生产环境中配置HTTPS
4. **定期更新**：定期更新系统和依赖库

## 版本更新

### 历史版本

- v1.0.0 (2023-10-15)：初始版本发布
  - 基本测量功能
  - Web界面支持
  - 校准功能

## 联系方式

如有问题或建议，请通过以下方式联系：
- Email: contact@traeai.com
- GitHub: https://github.com/traeai

## 许可证

MIT License

---

**注意：这是一个开源项目，仅供学习和参考。在实际应用中，请确保符合相关安全规定和标准。**
