<template>
    <MainContent>
        <div class="extension-content">
            <el-col :span="10" style="height: 100%">
                <extension-list>

                </extension-list>
            </el-col>

            <el-col :span="14">
                222
            </el-col>
        </div>
    </MainContent>
</template>
<script>

import MainContent from '@/components/Layout/MainContent';
import ExtensionList from './components/ExtensionList';

export default {
    components: {
        MainContent,
        ExtensionList
    }
}
</script>
<style lang="scss">
.extension-content{
    height: 100%;
}
.extension-content .tr-dashboard{
    padding-bottom: 0px;
}
</style>

