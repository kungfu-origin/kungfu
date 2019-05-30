//大于零
export const biggerThanZeroValidator = (rule,value,callback) => {
    if(value === '' || value == null){
        callback()
        return;
    }

    if(+value <= 0){
        callback(new Error('必须大于零！'))
    }else{
        callback();
    }
}

//不能是特殊字符
export const specialStrValidator = (rule, value, callback) => {
    const reg = new RegExp("[`~!@#$^&*()=|{}':;',\\[\\].<>《》/?~！@#￥……&*（）——|{}【】‘；：”“'。，、？]")
    if(reg.test(value)) {
        callback(new Error('不能包含特殊字符！'))
    }else{
        callback()
    }
}

//不能以0开头
export const noZeroAtFirstValidator = (rule, value, callback) => {
    if(value[0].toString() === '0') {
        callback(new Error('不能以数字0开头！'))
    }else{
        callback()
    }
}

//不能有中文
export const chineseValidator = (rule, value, callback) => {
    const reg = new RegExp("[\u4e00-\u9fa5]")
    if(reg.test(value)) {
        callback(new Error('不能包含中文！'))
    }else{
        callback()
    }
}

//不能含有空格
export const blankValidator = (rule, value, callback) => {
    const reg = new RegExp(/\s+/g)
    if(reg.test(value)) {
        callback(new Error('不能有空格！'))
    }else{
        callback()
    }
}

 //check 只能字母数字
export const validateNumberAndWords = (rule, value, callback) => {
    const p = /^[A-Za-z0-9]+$/;
    if(!p.test(value)){
        callback(new Error('策略名称只能包含字母与数字！'))
    }else{
        callback()
    }
}

//必须以tcp://开头
export const validateTCPUri = (rule, value, callback) => {
    const keyword = 'tcp://';
    if(value && (value.indexOf(keyword) === -1 || value.indexOf(keyword) !== 0)) {
        callback(new Error('uri 必须以 “tcp://” 开头！'))
    }else{
        callback()
    }
}

//0-99
export const validate099 = (rule, value, callback) => {
    if(value && (value < 1 || value > 99)) {
        callback(new Error('必须是 1～99 内整数！'))
    }else{
        callback()
    }
}

//整数
export const validateInt = (rule, value, callback) => {
    if(value && value % 1 !== 0) {
        callback(new Error('必须是整数！'))        
    }else{
        callback()
    }
}
