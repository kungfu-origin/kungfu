<template>
    <MainContent>

        <div class="market-filter-content">
            <el-row style="height: 55%">
                <el-col :span="4">
                    <TickerSet></TickerSet>
                </el-col>
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import MainContent from '@/components/Layout/MainContent';
import TickerSet from '@/components/MarketFilter/components/TickerSet';

export default {

    components: {
        MainContent,
        TickerSet
    }
}
</script>

<style lang="scss" scoped>

.market-filter-content {
    height: 100%
}

.el-col {
    height: 100%;
}

</style>