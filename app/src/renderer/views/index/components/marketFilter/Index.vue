<template>
    <MainContent>

        <div class="kf-market-filter__warp">
            <el-row style="height: 55%">
                <el-col :span="10">
                    <Tickers></Tickers>
                </el-col>
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import MainContent from '@/components/Layout/MainContent';
import Tickers from '@/components/MarketFilter/components/Tickers';

export default {

    components: {
        MainContent,
        Tickers
    }
}
</script>

<style>

</style>