export default [
    {
        key: "id",
        name: "ID",
        type: "readonly"
    },
    {
        key: "nickname",
        name: "昵称",
        type: "str",
        required: true,
    },
    {
        key: "email",
        name: "邮箱",
        type: "readonly"
    },
    {
        key: "phone",
        name: "手机号",
        type: "readonly"
    },
    {
        key: "gender",
        name: "性别",
        type: "select",
        options: [
            {
                name: "-",
                value: "U"
            },
            {
                name: "男",
                value: "F"
            },{
                name: "女",
                value: "M"
            }
        ]
    }, 
    {
        key: "birthdate",
        name: "生日",
        type: "date"
    },
    {
        key: "address",
        name: "所在地",
        type: "str",
    },
    {
        key: "company",
        name: "公司名",
        type: "str"
    } 
]