import { NMSG_PUB_FILE, NMSG_REP_FILE } from '__gConfig/pathConfig';
const kfcore = require('kungfu-core');
const nano = kfcore.nanomsg;
/**
 * 重新开启nano，必须把之前的nano close掉，为了防止重复监听
 * 对于pos/order/trades的nanomsg 监听要跟gloabel gateway划分开，防止污染
 * **/

export const buildSubNmsg = () => {
    const sub = nano.socket('sub');
    const addr = `ipc://${NMSG_PUB_FILE}`;
    sub.connect(addr)
    return sub
}


export const buildRepNmsg = () => {
    const req = nano.socket('req');
    const addr = `ipc://${NMSG_REP_FILE}`;
    req.connect(addr)
    return req
}

















































































