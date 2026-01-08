#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SWR驻波表 - Raspberry Pi版本主程序
用于测量SWR值、正向功率和反射功率，并提供Web访问接口
"""

import RPi.GPIO as GPIO
import time
import threading
import http.server
import socketserver
import json
import os

# 配置参数
FWD_PIN = 0  # 正向功率传感器模拟输入引脚
REV_PIN = 1  # 反射功率传感器模拟输入引脚

# 校准系数
CAL_FWD = 1.0
CAL_REV = 1.0

# 全局变量
swr_data = {
    "forward_power": 0.0,
    "reverse_power": 0.0,
    "swr": 1.0,
    "reflection_coefficient": 0.0,
    "power_loss": 0.0,
    "timestamp": time.time()
}

lock = threading.Lock()


def setup_gpio():
    """初始化GPIO"""
    GPIO.setmode(GPIO.BCM)
    # 设置传感器引脚为输入
    GPIO.setup(FWD_PIN, GPIO.IN)
    GPIO.setup(REV_PIN, GPIO.IN)
    print("GPIO初始化完成")


def read_analog(pin):
    """读取模拟输入值（需要ADC模块支持）"""
    # 这里使用模拟读取的示例代码，实际需要根据使用的ADC模块调整
    # 例如使用MCP3008等ADC模块
    try:
        # 模拟读取值（0-1023）
        value = 0  # 实际项目中替换为真实的ADC读取代码
        return value
    except Exception as e:
        print(f"读取模拟值失败: {e}")
        return 0


def calculate_power(adc_value, cal_factor):
    """将ADC值转换为功率值"""
    # 简单的线性转换示例，实际需要根据传感器特性校准
    voltage = adc_value * (3.3 / 1023.0)
    power = voltage * voltage * cal_factor
    return power


def calculate_swr(fwd_power, rev_power):
    """计算SWR值"""
    if rev_power == 0:
        return 1.0
    if fwd_power < rev_power:
        return 10.0  # 最大显示10
    
    rho = (rev_power / fwd_power) ** 0.5
    swr = (1 + rho) / (1 - rho)
    return round(swr, 2)


def calculate_reflection_coefficient(fwd_power, rev_power):
    """计算反射系数"""
    if rev_power == 0:
        return 0.0
    if fwd_power < rev_power:
        return 1.0
    
    rho = (rev_power / fwd_power) ** 0.5
    return round(rho, 4)


def calculate_power_loss(reflection_coefficient):
    """计算功率损失百分比"""
    loss = (1 - (1 - reflection_coefficient ** 2)) * 100
    return round(loss, 2)


def measure_loop():
    """测量循环"""
    global swr_data
    
    while True:
        try:
            # 读取传感器值
            fwd_adc = read_analog(FWD_PIN)
            rev_adc = read_analog(REV_PIN)
            
            # 计算功率
            fwd_power = calculate_power(fwd_adc, CAL_FWD)
            rev_power = calculate_power(rev_adc, CAL_REV)
            
            # 计算SWR和其他参数
            swr = calculate_swr(fwd_power, rev_power)
            reflection_coef = calculate_reflection_coefficient(fwd_power, rev_power)
            power_loss = calculate_power_loss(reflection_coef)
            
            # 更新全局数据
            with lock:
                swr_data = {
                    "forward_power": round(fwd_power, 2),
                    "reverse_power": round(rev_power, 2),
                    "swr": swr,
                    "reflection_coefficient": reflection_coef,
                    "power_loss": power_loss,
                    "timestamp": time.time()
                }
            
            time.sleep(0.5)  # 500ms采样一次
            
        except Exception as e:
            print(f"测量过程中发生错误: {e}")
            time.sleep(1)


class SWRHandler(http.server.SimpleHTTPRequestHandler):
    """自定义HTTP请求处理器"""
    
    def do_GET(self):
        """处理GET请求"""
        if self.path == '/api/swr':
            # 返回SWR数据
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            with lock:
                data = swr_data.copy()
                data['timestamp'] = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(data['timestamp']))
            
            self.wfile.write(json.dumps(data).encode('utf-8'))
        elif self.path == '/api/config':
            # 返回配置信息
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            config = {
                "calibration": {
                    "forward": CAL_FWD,
                    "reverse": CAL_REV
                },
                "pins": {
                    "forward": FWD_PIN,
                    "reverse": REV_PIN
                }
            }
            
            self.wfile.write(json.dumps(config).encode('utf-8'))
        else:
            # 静态文件服务
            super().do_GET()


def start_web_server():
    """启动Web服务器"""
    PORT = 8080
    
    # 确保在项目根目录下提供文件服务
    os.chdir(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
    
    with socketserver.TCPServer(("", PORT), SWRHandler) as httpd:
        print(f"Web服务器启动在端口 {PORT}")
        print(f"访问地址: http://localhost:{PORT}")
        print(f"API接口: http://localhost:{PORT}/api/swr")
        httpd.serve_forever()


def cleanup():
    """清理资源"""
    GPIO.cleanup()
    print("资源清理完成")


if __name__ == "__main__":
    try:
        print("SWR驻波表 - Raspberry Pi版本")
        setup_gpio()
        
        # 启动测量线程
        measure_thread = threading.Thread(target=measure_loop, daemon=True)
        measure_thread.start()
        
        # 启动Web服务器
        start_web_server()
        
    except KeyboardInterrupt:
        print("\n程序被用户中断")
    finally:
        cleanup()
