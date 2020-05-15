<template>
    <main-content
		:ifSideBar="false"
		:ifFooter="false"
	>
        <div class="trader-content">
                <el-col :span="14">
                    <el-row style="height: 33.33%">
                        <el-col :span="14">

                        </el-col>
                        <el-col :span="10">
                            <Pnl
                                ref="pnl"
                                :currentId="strategyId" 
                                moduleType="strategy"
                                :minPnl="pnl"   
                                :dailyPnl="dailyPnl"
                                :addTime="addTime"                
                                ></Pnl>
                        </el-col>
                    </el-row>
                    <el-row style="height: 66.66%">
                        <el-col>
                            <Log name="回测日志"></Log>                         
                        </el-col>
                    </el-row>
                </el-col>
                <el-col  :span="10">
                    <el-row style="height: 33.333%">
                            <Pos
							name="回测持仓"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :kungfuData="positions"   
                            :addTime="addTime"     
							:ifBacktest="true"           
                            ></Pos>
                    </el-row>
                    <el-row  style="height: 33.333%">
                            <CurrentOrder
							name="回测委托"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :kungfuData="orders"                 
                            :addTime="addTime"   
                            :orderStat="orderStat"     
							:ifBacktest="true"           
                            ></CurrentOrder>                      
                    </el-row>
                    <el-row style="height: 33.333%">
                            <TradeRecord 
							name="回测成交"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :kungfuData="trades"           
                            :addTime="addTime"    
                            :orderStat="orderStat"
							:ifBacktest="true"           
                            ></TradeRecord>
                    </el-row>
                </el-col>

        </div>
    </main-content>
</template>

<script>

import Log from '../Strategy/components/Log';
import CurrentOrder from '../Base/tradingData/CurrentOrder';
import TradeRecord from '../Base/tradingData/TradeRecord';
import Pos from '../Base/tradingData/Pos';
import Pnl from '../Base/tradingData/pnl/Index';

export default {

    data(){
        this.tradingDataPipe = null;
        return {
			addTime: 0,

            orders: Object.freeze([]),
            trades: Object.freeze([]),
            positions: Object.freeze([]),
            pnl: Object.freeze([]),
            dailyPnl: Object.freeze([]),
            orderStat: Object.freeze({})
        }
	},

	computed: {
		strategyId () {
			return this.$route.params.id
		}
	},
	
	components: {
        CurrentOrder, TradeRecord, Pos, Log, Pnl 
    },

}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.trader-content{
    height: 100%;
}

.trader-content > .el-row{
    height: 100%;
}

.el-col{
    height: 100%;
}

.el-row > .tr-dashboard{
    padding-right: 0px;
}

.trader-content > .el-row:last-child .tr-dashboard{
    padding-bottom: 0px;
}

.el-col > .el-row:last-child .tr-dashboard{
    padding-bottom: 0px;
}
.el-col.has-padding .tr-dashboard{
    padding-bottom: 8px !important;
}
</style>
