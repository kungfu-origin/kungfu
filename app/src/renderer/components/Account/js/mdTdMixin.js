//type: 'md' / 'td'
export default {
    data () {
        return  {
            method: 'add',
            accountForm: {},
            selectedSource: '',
            visiblity: {
                selectSource: false,
                setAccount: false,
            },

            renderTable: false
        }
    },

    mounted () {
        const t = this;
        t.renderTable = true;
        t.getTableList();
    },

    methods: {
        //添加账户，打开选择柜台弹窗
        handleAdd () {
            this.visiblity.selectSource = true;
        },

        //编辑账户
        handleOpenUpdateAccountDialog (row) {
            const t = this
            t.method = 'update'
            t.accountForm = JSON.parse(row.config) 
            t.selectedSource = row.source_name
            t.visiblity.setAccount = true
        },
        //选择柜台
        handleSelectSource (sourceName) {
            const t = this
            t.method = 'add';
            t.selectedSource = sourceName
            t.visiblity.setAccount = true
        },

        //添加或修改账户详情后的操作
        successSubmitSetting () {
            const t = this;
            t.getTableList();
            t.refreshData();
        },

        //清空数据
        refreshData () {
            const t = this
            t.selectedSource = ''
        },
    }
    
}