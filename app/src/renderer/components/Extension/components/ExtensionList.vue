<template>
    <tr-dashboard :noHeader="true">
        <div class="extension-list-content">
            <div class="search-keyword">
                <el-input placeholder="搜索插件"></el-input>
            </div>
        </div>
    </tr-dashboard>
</template>
<script>
import { getExtensions } from '__gUtils/busiUtils';
export default {
    name: 'extension-list',
    data() {
        return {
            extensionName: ''
        }
    },

    mounted() {
        const t = this;
        getExtensions();
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.extension-list-content{
    .search-keyword{
        padding: 5px;
        box-sizing: border-box;
        input {
            height: 40px;;
            background: $bg_dark;
            font-size: 16px;
        }
    }
}

</style>


