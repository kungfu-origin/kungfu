<template>
    <main-content>
        <div class="extension-content">
            <el-col :span="10" style="height: 100%">
                <extension-list>

                </extension-list>
            </el-col>

            <el-col :span="14">
                222
            </el-col>
        </div>
    </main-content>
</template>
<script>
import ExtensionList from './components/ExtensionList';
export default {
    components: {
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

