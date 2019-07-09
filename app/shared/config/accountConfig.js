import {blankValidator, validateTCPUri, validate099, validateInt} from '__assets/validator'
import {platform} from '__gConfig/platformConfig';

export const ifSourceDisable = {
    ctp: platform === 'mac',
    xtp: false
}

export const accountSource = {
    ctp: [
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
    ],
    xtp: [
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
    ],
    // oes: [
    //     {
    //         key: 'user_id',
    //         name: 'user_id',
    //         type: 'str',
    //         rule: '请填写user_id！',
    //         required: true,
    //     },
    //     {
    //         key: 'password',
    //         name: 'Password',
    //         type: 'str',
    //         rule: '请填写password！',
    //         required: true,
    //         validator: blankValidator //不能包含空格
    //     },
    //     {
    //         key: 'md_uri',
    //         name: 'md_uri',
    //         type: 'str',
    //         rule: '请填写md_uri！',
    //         required: true
    //     },
    //     {
    //         key: 'qry_uri',
    //         name: 'qry_uri',
    //         type: 'str',
    //         rule: '请填写qry_uri！',
    //         required: true
    //     },
    //     {
    //         key: 'ord_uri',
    //         name: 'ord_uri',
    //         type: 'str',
    //         rule: '请填写ord_uri！',
    //         required: true
    //     },
    //     {
    //         key: 'rpt_uri',
    //         name: 'rpt_uri',
    //         type: 'str',
    //         rule: '请填写rpt_uri！',
    //         required: true
    //     },
    // ],
    // sandbox: [
    //     {
    //         key: 'account_id',
    //         name: 'account_id',
    //         type: 'str',
    //         rule: '请填写account_id！',
    //         required: true,
    //     },
    //     {
    //         key: 'db_file',
    //         name: 'db_file',
    //         type: 'str',
    //         rule: '请填写db_file！',
    //         required: true,
    //     },
    //     {
    //         key: 'is_loop',
    //         name: 'is_loop',
    //         type: 'bool',
    //         rule: '请填写is_loop！',
    //         required: false,
    //         default: false
    //     },
    // ]
}

export const sourceType = {
    ctp: {
        source: 'ctp',
        type: 'danger',
        typeName: 'future',
        key:'account_id'
    },
    xtp: {
        source: 'xtp',
        type: '',
        typeName: 'stock',
        key:'user_id'
    },
    // oes: {
    //     source: 'oes',
    //     type: '',
    //     typeName: '股票',
    //     key:'user_id'
    // },
    // sandbox: {
    //     source: 'sandbox',
    //     type: 'success',
    //     typeName: '模拟',
    //     key:'account_id'
    // }
}
