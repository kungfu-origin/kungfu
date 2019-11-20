const moment = require("moment");

const isEnglish = process.env.LANG_ENV === 'en';

//大于零
export const biggerThanZeroValidator = (rule: any, value: any, callback: Function): void => {
    if(value === '' || value == null){
        callback()
        return;
    }

    if(+value <= 0){
        callback(new Error(
            isEnglish 
            ? 'Should bigger than zero!' 
            : '必须大于零！'))
    }else{
        callback();
    }
}

//不能是特殊字符
export const specialStrValidator = (rule: any, value: any, callback: Function): void => {
    const reg = new RegExp("[`~!@#$^&*()=|{}':;',\\[\\].<>《》/?~！@#￥……&*（）——|{}【】‘；：”“'。，、？]")
    if(reg.test(value)) {
        callback(new Error(
            isEnglish 
            ? 'Should exclude special characters!' 
            : '不能包含特殊字符！'))
    }else{
        callback()
    }
}

//不能以0开头
export const noZeroAtFirstValidator = (rule: any, value: any, callback: Function): void => {
    if(value[0].toString() === '0') {
        callback(new Error(
            isEnglish 
            ? 'Should not start with "0"!'
            : '不能以数字0开头！'))
    }else{
        callback()
    }
}

//不能有中文
export const chineseValidator = (rule: any, value: any, callback: Function): void => {
    const reg = new RegExp("[\u4e00-\u9fa5]")
    if(reg.test(value)) {
        callback(new Error(
            isEnglish 
            ? 'Should exclude chinese characters!'
            : '不能包含中文！'))
    }else{
        callback()
    }
}

//不能含有空格
export const blankValidator = (rule: any, value: any, callback: Function): void => {
    const reg = new RegExp(/\s+/g)
    if(reg.test(value)) {
        callback(new Error(
            isEnglish 
            ? 'Should exclude spaces!'
            : '不能有空格！'))
    }else{
        callback()
    }
}

 //check 只能字母数字
export const numberAndWordsValidator = (rule: any, value: any, callback: Function): void => {
    const p = /^[A-Za-z0-9]+$/;
    if(!p.test(value)){
        callback(new Error(
            isEnglish
            ? 'Should only include A-Za-z0-9!'
            : '策略名称只能包含字母与数字！'))
    }else{
        callback()
    }
}

//必须以tcp://开头
export const tcpUriValidator = (rule: any, value: any, callback: Function): void => {
    const keyword: string = 'tcp://';
    if(value && (value.indexOf(keyword) === -1 || value.indexOf(keyword) !== 0)) {
        callback(new Error(
            isEnglish
            ? 'Should start with "tcp://"!'
            : 'uri 必须以 “tcp://” 开头！'))
    }else{
        callback()
    }
}

//0-99
export const o99Validator = (rule: any, value: any, callback: Function): void => {
    if(!value || (value < 1 || value > 99)) {
        callback(new Error(
            isEnglish
            ? 'Should be integer between 1 and 99!'
            : '必须是 1～99 内整数！'))
    }else{
        callback()
    }
}

//整数
export const intValidator = (rule: any, value: any, callback: Function): void => {
    if(value && value % 1 !== 0) {
        callback(new Error(
            isEnglish
            ? 'Should be integer!'
            : '必须是整数！'))        
    }else{
        callback()
    }
}

//必填
export const requiredValidator = (rule: any, value: any, callback: Function): void => {
    if(!(value + '') || (value.toString().trim() === 'NaN')) {
        callback(new Error(
            isEnglish
            ? 'Required!'
            : '不能为空！'))
    } else {
        callback()       
    }
}

export const dateFormatYYMMDDValidator = (rule: any, value: any, callback: Function): void => {
    if(moment(value.toString(), 'YYMMDD').format('YYMMDD') === 'Invalid date') {
        callback(new Error(
            isEnglish
            ? 'Date format should be YYMMDD '
            : '日期格式YYMMDD！'))
    }
}