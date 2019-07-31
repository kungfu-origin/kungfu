import {blankValidator, validateTCPUri, validate099, validateInt} from '__assets/validator'
import {platform} from '__gConfig/platformConfig';

export const ifSourceDisable = {
    ctp: platform === 'mac',
    xtp: false
}

const tagColor = {
    'future': 'danger', //red,
    'stock': ''
};

export const accountSource = {
    ctp: {
        source: 'ctp',
        type: tagColor['future'] || '',
        typeName: 'future',
        key:'account_id',
        config: [
            {
                key: 'account_id',
                name: 'account_id',
                type: 'str',
                rule: '请填写用户account_id！',
                required: true,
            },
            {
                key: 'password',
                name: 'password',
                type: 'password',
                rule: '请填写password！',
                required: true,
                validator: [blankValidator] //不能包含空格
            },
            {
                key: 'broker_id',
                name: 'broker_id',
                type: 'str',
                rule: '请填写broker_id！',
                required: true
            },
            {
                key: 'md_uri',
                name: 'md_uri',
                type: 'str',
                rule: '请填写md_uri！',
                required: true,
                validator: [validateTCPUri]
            },
            {
                key: 'td_uri',
                name: 'td_uri',
                type: 'str',
                rule: '请填写td_uri！',
                required: true,
                validator: [validateTCPUri]
            }
        ]
    },
    xtp: {
        source: 'xtp',
        type: tagColor['stock'] || '',
        typeName: 'stock',
        key:'user_id',
        config: [
            {
                key: 'user_id',
                name: 'user_id',
                type: 'str',
                rule: '请填写user_id！',
                required: true,
            },
            {
                key: 'password',
                name: 'password',
                type: 'password',
                rule: '请填写password！',
                required: true,
                validator: [blankValidator] //不能包含空格
            },
            {
                key: 'software_key',
                name: 'software_key',
                type: 'str',
                rule: '请填写software_key！',
                required: true
            },
            {
                key: 'md_ip',
                name: 'md_ip',
                type: 'str',
                rule: '请填写md_ip！',
                required: true
            },
            {
                key: 'md_port',
                name: 'md_port',
                type: 'int',
                rule: '请填写md_port！',
                validator: [validateInt],
                required: true
            },
            {
                key: 'td_ip',
                name: 'td_ip',
                type: 'str',
                rule: '请填写td_ip！',
                required: true
            },
            {
                key: 'td_port',
                name: 'td_port',
                type: 'int',
                rule: '请填写td_port！',
                validator: [validateInt],
                required: true
            },
            {
                key: 'client_id',
                name: 'client_id',
                type: 'int',
                rule: '请填写0-99整数',
                required: false,
                validator: [validate099, validateInt],
                tip: '多点登陆Id，需是0～99内整数'
            },
        ]
    } 
}