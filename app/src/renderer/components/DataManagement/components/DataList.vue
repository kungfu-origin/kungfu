<!--
 * @Author: your name
 * @Date: 2020-06-22 14:13:23
 * @LastEditTime: 2020-06-22 18:10:27
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/src/renderer/components/DataManagement/components/DataList.vue
--> 
<template>
    <tr-dashboard title="数据列表">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <tr-search-input v-model.trim="databaseInput"></tr-search-input>
            </tr-dashboard-header-item>
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleAdd" title="添加">添加</el-button>
            </tr-dashboard-header-item>
        </div>
        <div class="kungfu-data-management-list-table__warp">
            <el-table
                v-if="renderTable"
                size="small"
                :data="dataList"
                height="100%"
                @row-click="handleRowClick"
                :row-class-name="handleSelectRow"
                >
                <el-table-column
                    prop="data_name"
                    label="名称"
                    show-overflow-tooltip
                    min-width="80"
                >
                </el-table-column>
                <el-table-column
                    prop="add_time"
                    label="添加时间"
                    min-width="80"
                    >
                </el-table-column>
·
                <el-table-column
                    label=""
                    align="right"
                    min-width="120"
                >
                    <template>
                        <span class="tr-oper"><i class="el-icon-document mouse-over" title="打开日志文件"></i></span>
                        <span class="tr-oper"><i class="el-icon-setting mouse-over" title="TD 设置"></i></span>
                        <span :class="['tr-oper-delete']"><i class=" el-icon-delete mouse-over" title="删除 TD"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </div>

    <el-dialog 
    width="340px" 
    :title="setDataSeriesDialogType == 'add' ? '添加数据集' : '设置数据集'"  
    v-if="setDataSeriesIdDialogVisiblity"
    :visible.sync="setDataSeriesIdDialogVisiblity" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClearAddDataSeriesDialog"
    @keyup.enter.native="handleConfirmAddDataSeries"
    >
        <el-form ref="setDataSeriesIdForm" label-width="90px" :model="setDataSeriesIdForm">
            <!-- 自定义部分 -->
            <el-form-item
                label="数据集ID"
                prop="dataSeriesId"
                :rules="[
                { required: true, message: '请输入数据集Id', trigger: 'blur' },
                { min: 1, max: 20, message: '长度不能超过 20 个字符', trigger: 'blur' },
                { validator: chineseValidator, trigger: 'blur' },
                { validator: specialStrValidator, trigger: 'blur' },
                { validator: noZeroAtFirstValidator, trigger: 'blur' },
                { validator: noKeywordValidatorBuilder('all'), trigger: 'blur' },
                { validator: noKeywordValidatorBuilder('ledger'), trigger: 'blur' }
                ]"
            >
                <el-input 
                v-model.trim="setDataSeriesIdForm.dataSeriesId" 
                :disabled="setDataSeriesDialogType == 'set'"
                 placeholder="请输入数据集名称"
                 ></el-input>
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer">
            <el-button  size="mini" @click="handleClearAddDataSeriesDialog">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleConfirmAddDataSeries">确 定</el-button>
        </div>
    </el-dialog>

    </tr-dashboard>
</template>

<script>

import { chineseValidator, specialStrValidator, noZeroAtFirstValidator, noKeywordValidatorBuilder } from '__assets/validator';

export default {

    data () {

        this.noKeywordValidatorBuilder = noKeywordValidatorBuilder;
        this.chineseValidator = chineseValidator;
        this.specialStrValidator = specialStrValidator;
        this.noZeroAtFirstValidator = noZeroAtFirstValidator;

        return  {
            databaseInput: '',
            renderTable: false,
            dataList: [],

            setDataSeriesIdForm: {
                dataSeriesId: ''
            },

            setDataSeriesIdDialogVisiblity: false
        }
    },

    mounted () {
        this.renderTable = true;
    },

    methods: {
        handleAdd () {
            this.setDataSeriesDialogType = 'add';
            this.setDataSeriesIdDialogVisiblity = true;
        },

        handleRowClick () {},

        handleSelectRow () {},

        handleClearAddDataSeriesDialog () {},

        handleConfirmAddDataSeries () {},
    }

}
</script>

<style lang="scss">

.kungfu-data-management-list-table__warp {
    height: 100%;
}

</style>