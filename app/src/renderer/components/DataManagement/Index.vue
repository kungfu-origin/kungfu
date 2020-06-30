<template>
     <main-content>
        <div class="kungfu-data-management__warp">
            <el-col style="width: 300px">
                <DataList @dataImportFinished="handleDataImportFinished"></DataList>
            </el-col>
            <el-col style="flex: 1">
                <DataView :currentId="currentDataSeriesId"></DataView>
            </el-col>
        </div>
    </main-content>
</template>

<script>

import DataList from './components/DataList';
import DataView from './components/DataView';

export default {

    data () {
        return {
            currentDataSeriesId: ''
        }
    },

    components: {
        DataView,
        DataList
    },

    methods: {
        handleDataImportFinished (currentId) {
            this.currentDataSeriesId = currentId
        }
    }
}

</script>

<style lang="scss">

.kungfu-data-management__warp {
    height: 100%;
    display: flex;

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