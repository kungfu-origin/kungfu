<template>
    <div class="info-list-content">
        <transition>        
            <div class="info-list" v-if="value">
                <el-tabs v-model.trim="currInfoType">
                    <el-tab-pane :lazy="true" label="今天" name="today">
                        <el-card class="info-card">
                            <div slot="header">
                                <span>柜台断线</span>
                                <span class="fr mouse-over">
                                    <i class="mouse-over fa fa-trash fa-lg" style="top: -2px"></i>
                                    清空
                                    </span>
                            </div>
                            <ul class="info-body">
                                <li v-for="item in getShowData(todayData.disconnection, todayShowHidden.disconnection)" :key="item.id">
                                    <div class="info-item-header">
                                        <div class="info-item-title fl">{{item.title}}</div>
                                        <div class="info-item-time fr">{{item.time}}</div>
                                    </div>
                                    <div class="info-item-body">{{item.data}}</div>
                                </li>
                                <li class="mouse-over" v-if="!todayShowHidden.disconnection && ifNeedShowHidden(todayData.disconnection)" @click="todayShowHidden.disconnection = true">显示更多...</li>
                                <li class="mouse-over" v-if="todayShowHidden.disconnection && ifNeedShowHidden(todayData.disconnection)" @click="todayShowHidden.disconnection = false">收起</li>
                                <li class="no-data" v-if="todayData.disconnection.length == 0">暂无数据...</li>
                            </ul>
                        </el-card>
                        <el-card class="info-card">
                            <div slot="header">
                                <span>交易任务</span>
                                <span class="fr mouse-over">
                                    <i class="mouse-over fa fa-trash fa-lg" style="top: -2px"></i>
                                    清空
                                    </span>
                            </div>
                            <ul class="info-body">
                                <li v-for="item in getShowData(todayData.executorError, todayShowHidden.executorError)" :key="item.id">
                                    <div class="info-item-header">
                                        <div class="info-item-title fl">{{item.title}}</div>
                                        <div class="info-item-time fr">{{item.time}}</div>
                                    </div>
                                    <div class="info-item-body">{{item.data}}</div>
                                </li>
                                <li class="mouse-over" v-if="!todayShowHidden.executorError && ifNeedShowHidden(todayData.executorError)" @click="todayShowHidden.executorError = true">显示更多...</li>
                                <li class="mouse-over" v-if="todayShowHidden.executorError && ifNeedShowHidden(todayData.executorError)" @click="todayShowHidden.executorError = false">收起</li>
                                <li class="no-data" v-if="todayData.executorError.length == 0">暂无数据...</li>                            
                            </ul>
                        </el-card>
                        <el-card class="info-card">
                            <div slot="header">
                                <span>风控报警</span>
                                <span class="fr mouse-over">
                                    <i class="mouse-over fa fa-trash fa-lg" style="top: -2px"></i>
                                    清空
                                    </span>
                            </div>
                            <ul class="info-body">
                                <li v-for="item in getShowData(todayData.riskControllerInfo, todayShowHidden.riskControllerInfo)" :key="item.id">
                                    <div class="info-item-header">
                                        <div class="info-item-title fl">{{item.title}}</div>
                                        <div class="info-item-time fr">{{item.time}}</div>
                                    </div>
                                    <div class="info-item-body">{{item.data}}</div>
                                </li>
                                <li class="mouse-over" v-if="!todayShowHidden.riskControllerInfo && ifNeedShowHidden(todayData.riskControllerInfo)" @click="todayShowHidden.riskControllerInfo = true">显示更多...</li>
                                <li class="mouse-over" v-if="todayShowHidden.riskControllerInfo && ifNeedShowHidden(todayData.riskControllerInfo)" @click="todayShowHidden.riskControllerInfo = false">收起</li>
                                <li class="no-data" v-if="todayData.riskControllerInfo.length == 0">暂无数据...</li>                            
                            </ul>
                        </el-card>

                    </el-tab-pane>




                    <el-tab-pane :lazy="true" label="历史" name="his">
                        <el-card class="info-card">
                            <div slot="header">
                                <span>柜台断线</span>
                                <span class="fr mouse-over">
                                    <i class="mouse-over fa fa-trash fa-lg" style="top: -2px"></i>
                                    清空
                                    </span>
                            </div>
                            <ul class="info-body">
                                <li v-for="item in getShowData(hisData.disconnection, hisShowHidden.disconnection)" :key="item.id">
                                    <div class="info-item-header">
                                        <div class="info-item-title fl">{{item.title}}</div>
                                        <div class="info-item-time fr">{{item.time}}</div>
                                    </div>
                                    <div class="info-item-body">{{item.data}}</div>
                                </li>
                                <li class="mouse-over" v-if="!hisShowHidden.disconnection && ifNeedShowHidden(hisData.disconnection)" @click="hisShowHidden.disconnection = true">显示更多...</li>
                                <li class="mouse-over" v-if="hisShowHidden.disconnection && ifNeedShowHidden(hisData.disconnection)" @click="hisShowHidden.disconnection = false">收起</li>
                                <li class="no-data" v-if="hisData.disconnection.length == 0">暂无数据...</li>
                            </ul>
                        </el-card>
                        <el-card class="info-card">
                            <div slot="header">
                                <span>交易任务</span>
                                <span class="fr mouse-over">
                                    <i class="mouse-over fa fa-trash fa-lg" style="top: -2px"></i>
                                    清空
                                    </span>
                            </div>
                            <ul class="info-body">
                                <li v-for="item in getShowData(hisData.executorError, hisShowHidden.executorError)" :key="item.id">
                                    <div class="info-item-header">
                                        <div class="info-item-title fl">{{item.title}}</div>
                                        <div class="info-item-time fr">{{item.time}}</div>
                                    </div>
                                    <div class="info-item-body">{{item.data}}</div>
                                </li>
                                <li class="mouse-over" v-if="!hisShowHidden.executorError && ifNeedShowHidden(hisData.executorError)" @click="hisShowHidden.executorError = true">显示更多...</li>
                                <li class="mouse-over" v-if="hisShowHidden.executorError && ifNeedShowHidden(hisData.executorError)" @click="hisShowHidden.executorError = false">收起</li>
                                <li class="no-data" v-if="hisData.executorError.length == 0">暂无数据...</li>                            
                            </ul>
                        </el-card>
                        <el-card class="info-card">
                            <div slot="header">
                                <span>风控报警</span>
                                <span class="fr mouse-over">
                                    <i class="mouse-over fa fa-trash fa-lg" style="top: -2px"></i>
                                    清空
                                    </span>
                            </div>
                            <ul class="info-body">
                                <li v-for="item in getShowData(hisData.riskControllerInfo, hisShowHidden.riskControllerInfo)" :key="item.id">
                                    <div class="info-item-header">
                                        <div class="info-item-title fl">{{item.title}}</div>
                                        <div class="info-item-time fr">{{item.time}}</div>
                                    </div>
                                    <div class="info-item-body">{{item.data}}</div>
                                </li>
                                <li class="mouse-over" v-if="!hisShowHidden.riskControllerInfo && ifNeedShowHidden(hisData.riskControllerInfo)" @click="hisShowHidden.riskControllerInfo = true">显示更多...</li>
                                <li class="mouse-over" v-if="hisShowHidden.riskControllerInfo && ifNeedShowHidden(hisData.riskControllerInfo)" @click="hisShowHidden.riskControllerInfo = false">收起</li>
                                <li class="no-data" v-if="hisData.riskControllerInfo.length == 0">暂无数据...</li>                            
                            </ul>
                        </el-card>

                    </el-tab-pane>
                </el-tabs>
            </div>
        </transition>
        <div class="info-list-mask"  v-if="value" @click="handleClose"></div>
    </div>    
</template>
<script>
export default {
    props: {
        value: Boolean
    },

    data(){
        return{
            todayData: {
                disconnection: [
                    {
                        id: 1,
                        title: 'asd',
                        time: '20181203',
                        data: '撒打算看撒风控来说倒海翻江阿莱克斯的发就流口水的faked佛拉到发旅客受到分哈伦裤货到付款啦地方'
                    },
                    {
                        id: 2,
                        title: 'asd',
                        time: '20181203',
                        data: 'asdasdasdasdasdasd'
                    },
                    {
                        id: 3,
                        title: 'asd',
                        time: '20181203',
                        data: 'asdasdasdasdasdasd'
                    },
                    {
                        id: 4,
                        title: 'asd',
                        time: '20181203',
                        data: 'asdasdasdasdasdasd'
                    }
                ],
                executorError: [],
                riskControllerInfo: [],
            },

            hisData: {
                disconnection: [],
                executorError: [],
                riskControllerInfo: [],
            },

            todayShowHidden: {
                disconnection: false,
                executorError: false,
                riskControllerInfo: false,
            },

            hisShowHidden: {
                disconnection: false,
                executorError: false,
                riskControllerInfo: false,
            },

            limit: 10,
            currInfoType: 'today'
        }
    },

    computed: {
        todayDisconnection(){
            const t = this;
        }
    },

    methods: {
        handleClose(){
            const t = this;
            t.$emit('input', false)
        },

        
        getShowData(data, showHidden){
            const t = this;
            if(showHidden) return data
            return data.length > t.limit ? data.slice(0, limit) : data
        },

        ifNeedShowHidden(data){
            const t = this;
            return data.length > t.limit
        }
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
    .info-list-content{
        .info-list-mask{
            position: fixed;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background: rgba(0,0,0,0.6)
        }
        .info-list{
            width: 440px;
            height: 100%;
            position: fixed;
            top: 0;
            right: 0;
            z-index: 100;
            background: $bg_card;
        }
        .info-list.v-enter{width: 0px;}
        .info-list.v-enter-active{transition:  all 0.3s}
        .info-list.v-enter-to{width: 440px;}
        .info-list.v-leave-active{transition:  all 0.2s}
        .info-list.v-leave-to{width: 0px; opacity: 0;}

        .el-tabs__item.is-top{
            width: 205px;
            text-align: center;
            box-sizing: border-box;
        }
        .info-body{
            li{
                padding: 5px 15px 15px;
                border-bottom: 1px solid $input_bg;
            }
            li.no-data{
                padding: 20px 10px;
            }
            li:last-child{
                border: none;
            }
            .info-item-header{
                height: 30px;
                line-height: 30px;
                font-size: 11px;
                color: $font;
            }
            .info-item-body{
                color: $font_5;
                font-size: 14px;
             
            }
        }
        .el-card{
            width: 400px;
            margin: 20px;
            .el-card__header{
                padding: 10px;
                background: $tab_header;
            }
            .el-card__body{
                padding: 0px 10px 10px;
                background: $bg_light;
            
            }
        }
    }
    
</style>

