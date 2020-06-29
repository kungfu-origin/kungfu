<!--
 * @Author: your name
 * @Date: 2020-06-22 14:15:01
 * @LastEditTime: 2020-06-22 14:16:38
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/src/renderer/components/DataManagement/components/dataView.vue
--> 
<template>
    <tr-dashboard :noHeader="true">
        <el-tabs type="border-card">
            <el-tab-pane label="概览">概览</el-tab-pane>
            <el-tab-pane label="数据明细">

            </el-tab-pane>
            <el-tab-pane label="可视化">可视化</el-tab-pane>
        </el-tabs>
    </tr-dashboard>
</template>

<script>

import { KF_DATASET_DIR } from '__gConfig/pathConfig';
import { kungfu } from '__gUtils/kungfuUtils';

const path = require('path');

export default {
    props: {
        currentId: String
    },

    watch: {
        currentId (newVal) {
            this.loadData(newVal)
        }
    },

    methods: {

        loadData (currentId) {
            const dataSetPath = path.join(KF_DATASET_DIR, currentId);
            console.log(dataSetPath, '---')
            const dataSource = kungfu.Assemble([ dataSetPath ])

            this.getDataSetStatus(dataSource)
                .then(() => {
                    const piece = dataSource.currentFrame().data();
                    console.log(piece)
                })

        },

        getDataSetStatus (dataSource) {
            return new Promise(resolve => {
                let timer = setInterval(() => {
                    if (dataSource.dataAvailable()) {
                        resolve(true)
                        clearInterval(timer)
                    }
                }, 1000)
            })
        } 
    }
}
</script>

<style>

</style>