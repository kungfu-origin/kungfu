<template>
     <main-content>
        <div class="kungfu-data-management__warp">
            <el-col :span="10">
                <DataList></DataList>
            </el-col>
            <el-col :span="14">
                <DataView></DataView>
            </el-col>
        </div>
    </main-content>
</template>

<script>

import DataList from './components/DataList';
import DataView from './components/DataView';

export default {

    components: {
        DataView,
        DataList
    }
}

</script>

<style lang="scss">

.kungfu-data-management__warp {
    height: 100%;

    .el-col {
        height: 100%;
        
        .tr-dashboard {
            padding-bottom: 0;
        }

        &:last-child {
            .tr-dashboard {
                padding-right: 0;
            }
        }
    }
}

</style>