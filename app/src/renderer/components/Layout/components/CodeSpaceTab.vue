<template>
    <div class="code-space-tab">
        <span class="code-space-tab-name" @click="handleClick">
            {{[code.tabSpaceType, code.tabSpaceSize].join(': ')}}
        </span>
        <tr-setting-dashboard
            v-if="spaceTabSettingVisiblity"
            :visible.sync="spaceTabSettingVisiblity"
            :infoList="currentSettingInfoList"
            @selected="handleSelected"
        >
        </tr-setting-dashboard>
    </div>        
</template>

<script>
import { mapState } from 'vuex';

export default {
    data() {
        return {
            spaceTabSettingVisiblity: false,
            currentSettingInfoList: [],
            codeSpaceSetting: [],
            
        }
    },
    
    computed: {
        ...mapState({
            code: state => state.BASE.kfConfig.code   
        })
    },

    methods: {
        //点击选择缩进
        handleClick() {
            const t = this;
            const type = t.code.tabSpaceType.toLowerCase();
            t.$store.dispatch('getKungfuConfig')
            t.clearState()
            t.currentSettingInfoList = [
                 {
                    name: 'Indent Using Spaces',
                    tip: type === 'Spaces' ? 'change view' : '',
                    key: 'Spaces',
                    active: type === 'Spaces'
                },
                {
                    name: 'Indent Using Tabs',
                    bottomLine: true,
                    tip: type === 'Tabs' ? 'change view' : '',
                    key: 'Tabs',
                    active: type === 'Tabs'
                }
            ]
            t.openSettingDashboard()
        },

        //选择后下一步
        handleSelected(item) {
            const t = this;
            const size = t.code.tabSpaceSize;
            const type = item.key;
            t.codeSpaceSetting.push(item)
            t.currentSettingInfoList = [
                {
                    name: 2,
                    key: 2,
                    tip: +size === 2 ? `configured ${type} size` : '',
                    active: +size === 2
                }, 
                {
                    name: 4,
                    key: 4,
                    tip: +size === 4 ? `configured ${type} size` : '',
                    active: +size === 4
                }
            ]

            if(t.codeSpaceSetting.length == 2) {
                const type = t.codeSpaceSetting[0].key;
                const size = t.codeSpaceSetting[1].key;
                t.$store.dispatch('setKungfuConfigByKeys', {
                    code: {
                        tabSpaceType: type,
                        tabSpaceSize: size
                    }
                })
                t.clearState()
                return
            };

            t.$nextTick()
            .then(() => {
                t.openSettingDashboard();
            })
        },

        openSettingDashboard() {
            const t = this;
            t.$nextTick()
            .then(() => {
                t.spaceTabSettingVisiblity = true
            })
        },

        clearState() {
            const t = this;
            t.codeSpaceSetting = [];

        }
    }
    
}
</script>

<style lang="scss">
    .code-space-tab-name{
        display: inline-block;
        width: 100%;
        text-align: center;
        font-size: 14px;
        padding-bottom: 2px;
        box-sizing: border-box;
    }
</style>