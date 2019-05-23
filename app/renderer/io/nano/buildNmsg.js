const nano = require('nanomsg');
const { BASE_DIR, GATEWAY_DIR, STRATEGY_DIR, SOCKET_DIR } = require('__gConfig/pathConfig');
const fse = require('fs-extra');
const path = require('path');
window.nanomsgController = {}

/**
 * 重新开启nano，必须把之前的nano close掉，为了防止重复监听
 * 对于pos/order/trades的nanomsg 监听要跟gloabel gateway划分开，防止污染
 * **/

//接收账户的相关推送
export const connectGatewayNanomsg = (gatewayName) => {
    if(!gatewayName) return false;
    const sub = nano.socket('sub');
    const ipcDir = path.join(GATEWAY_DIR, gatewayName);
    fse.ensureDirSync(ipcDir)
    const ipcPath = path.join(ipcDir, 'pub.ipc')
    const addr = `ipc://${ipcPath}`
    sub.connect(addr);
    return sub
}

//在策略模块的账户监听，与全局区别开，防止关闭/开启操作污染全局gatewaynano状态
export const refreshGatewayNanomsg = (gatewayName, oldNanomsg) => {
    if(oldNanomsg) {
        oldNanomsg.close();
        oldNanomsg = null;
    };
    return connectGatewayNanomsg(gatewayName)
}

//刷新全局gatewayNanomsg
export const refreshGlobalGatewayNanomsg = (gatewayName) => {
    window.globalGatewaysNanomsg = window.globalGatewaysNanomsg || {}
    if(window.globalGatewaysNanomsg[gatewayName]) closeGlobalGatewayNanomsg(gatewayName)
    const newNanomsg = connectGatewayNanomsg(gatewayName)
    window.globalGatewaysNanomsg[gatewayName] = newNanomsg
    return newNanomsg
}

//关闭全局gatewayNanomsg
export const closeGlobalGatewayNanomsg = (gatewayName) => {
    if(!(window.globalGatewaysNanomsg && window.globalGatewaysNanomsg[gatewayName])) return
    window.globalGatewaysNanomsg[gatewayName].close()
    window.globalGatewaysNanomsg[gatewayName] = null;
    delete window.globalGatewaysNanomsg[gatewayName]
    return true;
}

//接收策略的相关推送
export const connectStrategyNanomsg = (strategyId) => {
    if(!strategyId) return false;
    const sub = nano.socket('sub');
    const ipcDir = path.join(STRATEGY_DIR, strategyId);
    fse.ensureDirSync(ipcDir)
    const ipcPath = path.join(ipcDir, 'pub.ipc')
    const addr = `ipc://${ipcPath}`
    sub.connect(addr);
    return sub
}

//close old strategy nano msg & open new one
export const refreshStrategyNanomsg = (strategyId, oldNanomsg)=> {
    if(oldNanomsg) {
        oldNanomsg.close();
        oldNanomsg = null;
    };
    return connectStrategyNanomsg(strategyId)
}

//接收交易日的相关推送
export const connectCalendarNanomsg = () => {
    const sub = nano.socket('sub');
    const ipcDir = path.join(BASE_DIR, 'calendar');
    fse.ensureDirSync(ipcDir)
    const ipcPath = path.join(ipcDir, 'pub.ipc')
    const addr = `ipc://${ipcPath}`
    sub.connect(addr)
    return sub
}

//请求获取交易日
export const reqCalendarNanomsg = () => {
    const req = nano.socket('req', {
        rcvtimeo: 1000
    })
    const ipcDir = SOCKET_DIR //KF_HOME/socket
    fse.ensureDirSync(ipcDir)
    const ipcPath = path.join(ipcDir, 'service.sock')
    const addr = `ipc://${ipcPath}`
    req.connect(addr)
    return req
}


//手动下撤单(账户)
export const reqGatewayNanomsg = (gatewayName) => {
    const req = nano.socket('req', {
        rcvtimeo: 1000
    })
    const ipcDir = path.join(GATEWAY_DIR, gatewayName)
    fse.ensureDirSync(ipcDir)
    const ipcPath = path.join(ipcDir, 'rep.ipc');
    const addr = `ipc://${ipcPath}`
    req.connect(addr)
    return req
}

export const reqStrategyNanomsg = (strategyId) => {
    const req = nano.socket('req', {
        rcvtimeo: 1000
    })
    const ipcDir = path.join(STRATEGY_DIR, strategyId)
    fse.ensureDirSync(ipcDir)
    const ipcPath = path.join(ipcDir, 'rep.ipc');
    const addr = `ipc://${ipcPath}`
    req.connect(addr)
    return req
}
