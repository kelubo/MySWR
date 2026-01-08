// 简单的测试服务器，模拟Arduino SWR驻波表的JSON响应
const http = require('http');
const fs = require('fs');
const path = require('path');

// 模拟SWR数据
function getSWRData() {
    const forwardPower = Math.random() * 1000; // 0-1000W
    const reflectedPower = Math.random() * forwardPower; // 反射功率小于正向功率
    const reflectionCoeff = Math.sqrt(reflectedPower / forwardPower);
    const swrValue = (1 + reflectionCoeff) / (1 - reflectionCoeff);
    const powerLoss = (reflectedPower / forwardPower) * 100;

    return {
        forwardPower: parseFloat(forwardPower.toFixed(1)),
        reflectedPower: parseFloat(reflectedPower.toFixed(1)),
        swrValue: parseFloat(swrValue.toFixed(1)),
        reflectionCoeff: parseFloat(reflectionCoeff.toFixed(2)),
        powerLoss: parseFloat(powerLoss.toFixed(1))
    };
}

// 历史数据存储
const HISTORY_SIZE = 60;
const history = [];

// 报警配置
let alarmThreshold = 2.0;
let alarmEnabled = true;
let alarmTriggered = false;

// 添加历史数据点
function addHistoryPoint() {
    const data = getSWRData();
    const timestamp = Date.now();
    
    history.push({
        forwardPower: data.forwardPower,
        reflectedPower: data.reflectedPower,
        swrValue: data.swrValue,
        timestamp: timestamp
    });
    
    if (history.length > HISTORY_SIZE) {
        history.shift();
    }
    
    // 检测报警
    if (alarmEnabled && data.swrValue > alarmThreshold) {
        alarmTriggered = true;
    } else {
        alarmTriggered = false;
    }
    
    return data;
}

// 初始化历史数据
for (let i = 0; i < HISTORY_SIZE; i++) {
    addHistoryPoint();
}

// 创建HTTP服务器
const server = http.createServer((req, res) => {
    console.log(`接收到请求: ${req.url}`);

    // 设置CORS头
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.writeHead(200);
        res.end();
        return;
    }

    if (req.url === '/data') {
        // 返回JSON数据（Arduino格式 - 驼峰命名）
        const data = addHistoryPoint();
        res.writeHead(200, {'Content-Type': 'application/json'});
        res.end(JSON.stringify(data));
    } else if (req.url === '/api/swr') {
        // 返回JSON数据（Raspberry Pi格式 - 下划线命名）
        const data = addHistoryPoint();
        const piFormatData = {
            forward_power: data.forwardPower,
            reverse_power: data.reflectedPower,
            swr: data.swrValue,
            reflection_coefficient: data.reflectionCoeff,
            power_loss: data.powerLoss,
            timestamp: Date.now()
        };
        res.writeHead(200, {'Content-Type': 'application/json'});
        res.end(JSON.stringify(piFormatData));
    } else if (req.url === '/history') {
        // 返回历史数据
        res.writeHead(200, {'Content-Type': 'application/json'});
        res.end(JSON.stringify({history: history}));
    } else if (req.url === '/alarm') {
        // 返回报警状态
        res.writeHead(200, {'Content-Type': 'application/json'});
        res.end(JSON.stringify({
            threshold: alarmThreshold,
            enabled: alarmEnabled,
            triggered: alarmTriggered
        }));
    } else if (req.url === '/control' && req.method === 'POST') {
        // 处理控制命令
        let body = '';
        req.on('data', chunk => {
            body += chunk.toString();
        });
        req.on('end', () => {
            try {
                const command = JSON.parse(body);
                console.log('接收到控制命令:', command);
                
                // 简单的命令处理
                let response = {success: true, message: ''};
                if (command.command === 'reset') {
                    response.message = 'Device reset';
                } else if (command.command === 'calibrate') {
                    response.message = 'Device calibrated';
                } else if (command.command === 'save_settings') {
                    response.message = 'Settings saved';
                } else {
                    response.success = false;
                    response.message = 'Unknown command';
                }
                
                res.writeHead(200, {'Content-Type': 'application/json'});
                res.end(JSON.stringify(response));
            } catch (error) {
                res.writeHead(400, {'Content-Type': 'application/json'});
                res.end(JSON.stringify({success: false, message: 'Invalid JSON'}));
            }
        });
    } else if (req.url === '/') {
        // 返回HTML页面
        fs.readFile(path.join(__dirname, 'index.html'), 'utf8', (err, data) => {
            if (err) {
                res.writeHead(500);
                res.end('读取HTML文件失败');
                return;
            }
            res.writeHead(200, {'Content-Type': 'text/html'});
            res.end(data);
        });
    } else if (req.url === '/styles.css') {
        // 返回CSS文件
        fs.readFile(path.join(__dirname, 'styles.css'), 'utf8', (err, data) => {
            if (err) {
                res.writeHead(500);
                res.end('读取CSS文件失败');
                return;
            }
            res.writeHead(200, {'Content-Type': 'text/css'});
            res.end(data);
        });
    } else if (req.url === '/script.js') {
        // 返回JavaScript文件
        fs.readFile(path.join(__dirname, 'script.js'), 'utf8', (err, data) => {
            if (err) {
                res.writeHead(500);
                res.end('读取JavaScript文件失败');
                return;
            }
            res.writeHead(200, {'Content-Type': 'text/javascript'});
            res.end(data);
        });
    } else {
        res.writeHead(404);
        res.end('页面未找到');
    }
});

// 启动服务器
const PORT = process.env.PORT || 8082;
server.listen(PORT, () => {
    console.log(`测试服务器运行在 http://localhost:${PORT}`);
    console.log(`请在浏览器中访问 http://localhost:${PORT} 测试远程访问功能`);
});
