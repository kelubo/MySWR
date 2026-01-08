// SWR驻波表模拟器

// DOM元素
const forwardPowerInput = document.getElementById('forwardPower');
const reflectedPowerInput = document.getElementById('reflectedPower');
const swrValueElement = document.getElementById('swrValue');
const calculatedSWR = document.getElementById('calculatedSWR');
const reflectionCoeff = document.getElementById('reflectionCoeff');
const powerLoss = document.getElementById('powerLoss');
const forwardValueDisplay = document.getElementById('forwardValue'); // 正向功率数字显示
const reflectedValueDisplay = document.getElementById('reflectedValue'); // 反向功率数字显示
const swrSection = document.querySelector('.swr-section'); // SWR区域元素
const statusGood = document.getElementById('statusGood');
const statusFair = document.getElementById('statusFair');
const statusPoor = document.getElementById('statusPoor');
const statusBad = document.getElementById('statusBad');

// 用户认证相关元素
const loginModal = document.getElementById('loginModal');
const loginBtn = document.getElementById('loginBtn');
const usernameInput = document.getElementById('username');
const passwordInput = document.getElementById('password');
const loginError = document.getElementById('loginError');
const currentUser = document.getElementById('currentUser');
const appContainer = document.getElementById('appContainer');

// 功能入口相关元素
const manageBtn = document.getElementById('manageBtn');
const historyBtn = document.getElementById('historyBtn');
const alarmSection = document.querySelector('.alarm-section');
const historySection = document.querySelector('.history-section');
const controlSection = document.querySelector('.control-section');

// 报警功能相关元素
const alarmThresholdInput = document.getElementById('alarmThreshold');
const alarmEnabledCheckbox = document.getElementById('alarmEnabled');
const alarmStatus = document.getElementById('alarmStatus');
const alarmIndicator = document.getElementById('alarmIndicator');
const alarmMessage = document.getElementById('alarmMessage');

// 历史数据功能已移除

// 远程控制相关元素
const resetBtn = document.getElementById('resetBtn');
const calibrateBtn = document.getElementById('calibrateBtn');
const saveSettingsBtn = document.getElementById('saveSettings');
const deviceStatus = document.getElementById('deviceStatus');
const updateIntervalInput = document.getElementById('updateInterval');

// 全局变量
let remoteMode = false;
let isAuthenticated = false;
let currentUsername = '';
let updateInterval = 1000;
let updateTimer = null;

// 计算SWR值
function calculateSWR(forward, reflected) {
    if (forward === 0) return Infinity;
    if (reflected >= forward) return Infinity;
    
    const ratio = Math.sqrt(reflected / forward);
    return (1 + ratio) / (1 - ratio);
}

// 计算反射系数
function calculateReflectionCoeff(forward, reflected) {
    if (forward === 0) return 0;
    return Math.sqrt(reflected / forward);
}

// 计算功率损失百分比
function calculatePowerLoss(forward, reflected) {
    if (forward === 0) return 0;
    return (reflected / forward) * 100;
}

// 更新状态指示器
function updateStatus(swr) {
    // 重置所有状态
    statusGood.classList.remove('active');
    statusFair.classList.remove('active');
    statusPoor.classList.remove('active');
    statusBad.classList.remove('active');
    
    // 重置SWR边框和数值颜色
    swrSection.style.borderColor = '#ddd';
    swrValueElement.style.color = '#2c3e50';
    
    if (swr <= 1.5) {
        statusGood.classList.add('active');
        swrSection.style.borderColor = '#27ae60'; // 绿色
        swrValueElement.style.color = '#27ae60';
    } else if (swr <= 2) {
        statusFair.classList.add('active');
        swrSection.style.borderColor = '#f39c12'; // 橙色
        swrValueElement.style.color = '#f39c12';
    } else if (swr <= 3) {
        statusPoor.classList.add('active');
        swrSection.style.borderColor = '#e67e22'; // 深橙色
        swrValueElement.style.color = '#e67e22';
    } else {
        statusBad.classList.add('active');
        swrSection.style.borderColor = '#e74c3c'; // 红色
        swrValueElement.style.color = '#e74c3c';
    }
}

// 更新所有显示
function updateDisplay() {
    const forward = parseFloat(forwardPowerInput.value) || 0;
    const reflected = parseFloat(reflectedPowerInput.value) || 0;
    
    // 确保反射功率不超过正向功率
    if (reflected > forward) {
        reflectedPowerInput.value = forward;
    }
    
    const swr = calculateSWR(forward, reflected);
    const coeff = calculateReflectionCoeff(forward, reflected);
    const loss = calculatePowerLoss(forward, reflected);
    
    // 更新数字表头显示
    forwardValueDisplay.textContent = forward.toFixed(1);
    reflectedValueDisplay.textContent = reflected.toFixed(1);
    
    // 更新SWR值显示
    if (swr === Infinity) {
        swrValueElement.textContent = '∞';
        calculatedSWR.textContent = '∞';
    } else {
        const swrDisplay = swr.toFixed(1);
        swrValueElement.textContent = swrDisplay;
        calculatedSWR.textContent = swrDisplay;
    }
    
    // 更新反射系数
    reflectionCoeff.textContent = coeff.toFixed(2);
    
    // 更新功率损失
    powerLoss.textContent = loss.toFixed(1) + '%';
    
    // 更新状态指示器
    updateStatus(swr);
}

// 事件监听器
forwardPowerInput.addEventListener('input', updateDisplay);
reflectedPowerInput.addEventListener('input', updateDisplay);

// 初始化显示
updateDisplay();

// 远程访问功能

// 检测是否在远程服务器上运行（Arduino或Raspberry Pi）
function isRemoteMode() {
    // 如果页面是从Arduino或Raspberry Pi服务器加载的
    return window.location.hostname === '192.168.1.177' || 
           window.location.hostname === 'localhost' && window.location.port === '80' ||
           window.location.hostname === 'localhost' && window.location.port === '8080' ||
           window.location.hostname === 'localhost' && window.location.port === '8082';
}

// 检查是否在远程模式下运行
remoteMode = isRemoteMode();

// 用户认证功能
function login() {
    const username = usernameInput.value.trim();
    const password = passwordInput.value.trim();
    
    // 简单的本地认证（实际应用中应该使用服务器端认证）
    if (username === 'admin' && password === 'password') {
        isAuthenticated = true;
        currentUsername = username;
        loginModal.style.display = 'none';
        currentUser.textContent = username;
        loginError.textContent = '';
    } else {
        loginError.textContent = '用户名或密码错误';
    }
}

// 功能入口处理函数
function toggleManageInterface() {
    // 检查用户是否已认证
    if (!isAuthenticated) {
        alert('请先登录后再进入管理界面');
        loginModal.style.display = 'block';
        return;
    }
    
    // 切换管理界面显示状态
    const isVisible = alarmSection.style.display !== 'none' || controlSection.style.display !== 'none';
    
    if (isVisible) {
        // 隐藏管理界面
        alarmSection.style.display = 'none';
        controlSection.style.display = 'none';
        manageBtn.textContent = '管理界面';
    } else {
        // 显示管理界面
        alarmSection.style.display = 'block';
        controlSection.style.display = 'block';
        manageBtn.textContent = '关闭管理';
        // 隐藏历史数据界面
        historySection.style.display = 'none';
        historyBtn.textContent = '历史数据';
    }
}

function toggleHistoryInterface() {
    // 检查用户是否已认证
    if (!isAuthenticated) {
        alert('请先登录后再查看历史数据');
        loginModal.style.display = 'block';
        return;
    }
    
    // 切换历史数据界面显示状态
    const isVisible = historySection.style.display !== 'none';
    
    if (isVisible) {
        // 隐藏历史数据界面
        historySection.style.display = 'none';
        historyBtn.textContent = '历史数据';
    } else {
        // 显示历史数据界面
        historySection.style.display = 'block';
        historyBtn.textContent = '关闭历史';
        // 隐藏管理界面
        alarmSection.style.display = 'none';
        controlSection.style.display = 'none';
        manageBtn.textContent = '管理界面';
    }
}

// 历史数据功能
// 历史数据相关函数已移除

// 报警功能
function checkAlarm(swrValue) {
    const threshold = parseFloat(alarmThresholdInput.value);
    const enabled = alarmEnabledCheckbox.checked;
    
    if (!enabled) {
        alarmStatus.className = 'alarm-status';
        alarmIndicator.className = 'status-dot';
        alarmMessage.textContent = '报警已禁用';
        return;
    }
    
    if (swrValue > threshold) {
        alarmStatus.className = 'alarm-status error';
        alarmIndicator.className = 'status-dot';
        alarmMessage.textContent = `报警触发! SWR值 (${swrValue.toFixed(1)}) 超过阈值 (${threshold})`;
    } else {
        alarmStatus.className = 'alarm-status normal';
        alarmIndicator.className = 'status-dot';
        alarmMessage.textContent = `正常 (阈值: ${threshold})`;
    }
}

// 远程控制功能
function sendControlCommand(command) {
    if (!remoteMode) return;
    
    // 检查用户是否已认证
    if (!isAuthenticated) {
        alert('请先登录后再执行控制操作');
        loginModal.style.display = 'block';
        return;
    }
    
    fetch('/control', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ command: command })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert(`命令执行成功: ${command}`);
        } else {
            alert(`命令执行失败: ${command}`);
        }
    })
    .catch(error => {
        console.error('发送控制命令失败:', error);
        alert('发送控制命令失败');
    });
}

// 初始化应用
function initApp() {
    // 如果是远程模式，设置轮询
    if (remoteMode) {
        // 隐藏控制输入框
        const controls = document.querySelector('.controls');
        if (controls) {
            controls.style.display = 'none';
        }
        
        // 设置定时轮询
        updateInterval = parseInt(updateIntervalInput.value) || 1000;
        updateTimer = setInterval(fetchRemoteData, updateInterval);
        
        // 立即获取一次数据
        fetchRemoteData();
    }
}

// 增强的远程数据获取函数，支持Arduino和Raspberry Pi
function fetchRemoteData() {
    // 首先尝试使用Raspberry Pi的API端点
    fetch('/api/swr')
        .then(response => {
            if (!response.ok) {
                // 如果失败，尝试使用Arduino的API端点
                return fetch('/data');
            }
            return response;
        })
        .then(response => response.json())
        .then(data => {
            // 解析数据，兼容两种格式
            let forwardPower, reversePower, swrValue, reflectionCoeff, powerLoss;
            
            // 检查是哪种数据格式
            if (data.forward_power !== undefined) {
                // Raspberry Pi格式
                forwardPower = data.forward_power;
                reversePower = data.reverse_power;
                swrValue = data.swr;
                reflectionCoeff = data.reflection_coefficient;
                powerLoss = data.power_loss;
            } else {
                // Arduino格式
                forwardPower = data.forwardPower;
                reversePower = data.reflectedPower;
                swrValue = data.swrValue;
                reflectionCoeff = data.reflectionCoeff;
                powerLoss = data.powerLoss;
            }
            
            // 更新显示值
            document.getElementById('forwardValue').textContent = forwardPower.toFixed(1);
            document.getElementById('reflectedValue').textContent = reversePower.toFixed(1);
            
            // 更新SWR值
            const swr = (swrValue === "Infinity" || swrValue === Infinity) ? Infinity : parseFloat(swrValue);
            if (swr === Infinity) {
                document.getElementById('swrValue').textContent = '∞';
                document.getElementById('calculatedSWR').textContent = '∞';
            } else {
                document.getElementById('swrValue').textContent = swr.toFixed(1);
                document.getElementById('calculatedSWR').textContent = swr.toFixed(1);
            }
            
            // 更新反射系数
            document.getElementById('reflectionCoeff').textContent = reflectionCoeff.toFixed(2);
            
            // 更新功率损失
            document.getElementById('powerLoss').textContent = powerLoss.toFixed(1) + '%';
            
            // 更新状态指示器
            updateStatus(swr);
            
            // 检查报警
            checkAlarm(swr);
        })
        .catch(error => {
            console.error('获取远程数据失败:', error);
        });
}

// 事件监听器
loginBtn.addEventListener('click', login);
// 功能入口事件监听器
manageBtn.addEventListener('click', toggleManageInterface);
historyBtn.addEventListener('click', toggleHistoryInterface);
// 历史数据事件监听器已移除
alarmThresholdInput.addEventListener('input', () => {
    const threshold = parseFloat(alarmThresholdInput.value);
    const enabled = alarmEnabledCheckbox.checked;
    if (enabled) {
        alarmMessage.textContent = `报警已启用 (阈值: ${threshold})`;
    }
});
alarmEnabledCheckbox.addEventListener('change', () => {
    const enabled = alarmEnabledCheckbox.checked;
    const threshold = parseFloat(alarmThresholdInput.value);
    if (enabled) {
        alarmMessage.textContent = `报警已启用 (阈值: ${threshold})`;
        alarmStatus.className = 'alarm-status normal';
    } else {
        alarmMessage.textContent = '报警已禁用';
        alarmStatus.className = 'alarm-status';
    }
});
resetBtn.addEventListener('click', () => sendControlCommand('reset'));
calibrateBtn.addEventListener('click', () => sendControlCommand('calibrate'));
saveSettingsBtn.addEventListener('click', () => sendControlCommand('save_settings'));
updateIntervalInput.addEventListener('input', () => {
    const newInterval = parseInt(updateIntervalInput.value) || 1000;
    if (updateTimer && newInterval !== updateInterval) {
        updateInterval = newInterval;
        clearInterval(updateTimer);
        updateTimer = setInterval(fetchRemoteData, updateInterval);
    }
});

// 登录模态框点击外部关闭
window.addEventListener('click', (event) => {
    if (event.target === loginModal) {
        loginModal.style.display = 'none';
    }
});

// 回车键登录
usernameInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
        login();
    }
});

passwordInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
        login();
    }
});

// 初始化时不显示登录界面，仅在需要时显示
loginModal.style.display = 'none';

// 初始化应用
initApp();