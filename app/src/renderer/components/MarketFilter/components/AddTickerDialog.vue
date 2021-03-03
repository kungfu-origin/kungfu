<template>
 <el-dialog
        width="300px"
        :visible="visible" 
        title="添加标的"  
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        @close="handleClose"
        @keyup.enter.native="handleSubmitSetting"
    >
    <div class="ticker-dialog__warp">
        <el-form ref='tickerForm' :model="tickerForm" label-width="70px">
            <el-form-item
                label="标的"
                prop="instrumentId"
                :rules="[
                    { required: true, message: '请输入标的池名称', trigger: 'blur' },
                ]"
            >
                <el-autocomplete
                size="mini"
                ref="insturment-id-input"
                placeholder="请输入标的"
                v-model="tickerForm.instrumentId"
                :fetch-suggestions="querySearch"
                @select="e => handleSelectInstrumentId(e)"
                >
                    <template v-slot="{ item }">
                        <div class="make-order-instrument-ids__warp">
                            <div class="make-order-instrument-id-item">
                                <span class="ticker">{{ item.instrument_id }}</span>
                                <span class="name">{{ item.instrument_name }}</span>
                            </div>
                            <div class="make-order-instrument-id-item">
                                {{ (item.exchange_id || "").toUpperCase() }}
                            </div>
                        </div>
                    </template>
                </el-autocomplete>
            </el-form-item>
            <el-form-item
                label="交易所"
                prop="exchangeId"
                :rules="[
                    { required: true, message: '请选择交易所', trigger: 'change' },
                ]"
            >
                <el-select
                size="mini"
                v-model.trim="tickerForm.exchangeId"
                >
                <el-option
                    v-for="option in ExchangeIds"
                    :key="option.value"
                    :label="option.name"
                    :value="option.value"
                >
                </el-option>
                </el-select>
            </el-form-item>
            <el-form-item
                label="柜台"
                prop="source"
                :rules="[
                    { required: true, message: '请选择行情柜台', trigger: 'change' },
                ]"
            >
                <el-select
                size="mini"
                v-model.trim="tickerForm.source"
                >
                    <el-option
                        v-for="source in Object.keys(mdAccountSource)"
                        :key="source"
                        :label="source"
                        :value="source"
                    >
                        <span>{{ source }}</span>
                        <el-tag :type="mdAccountSource[source].type">{{
                        (SourceTypeConfig[mdAccountSource[source].typeName] || {}).name ||
                        ""
                        }}</el-tag>
                    </el-option>
                </el-select>
            </el-form-item>
        </el-form>
    </div>
            
    <div slot="footer" class="dialog-footer">
        <el-button  size="mini" @click="handleClose">取 消</el-button>
        <el-button type="primary" size="mini" @click="handleSubmitSetting">确 定</el-button>
    </div>

 </el-dialog>
    
</template>

<script>
import Vue from "vue";
import { mapState } from 'vuex';
import { Autocomplete } from "element-ui";

import {
  SourceTypeConfig,
  ExchangeIds,
} from "kungfu-shared/config/tradingConfig";
import { deepClone } from '__gUtils/busiUtils';

import instrumentsMixin from "@/assets/js/mixins/instrumentsMixin";

Vue.use(Autocomplete);

export default {

    mixins: [ instrumentsMixin ],

    props: {

        visible: {
            type: Boolean,
            default: false
        },

    },



    data () {
        this.SourceTypeConfig = SourceTypeConfig;
        this.ExchangeIds = Object.keys(ExchangeIds || {}).map((key) => {
            return {
                value: key,
                name: ExchangeIds[key],
            };
        });

        return {
            tickerForm: {
                instrumentId: '',
                exchangeId: '',
                source: ''
            },


            
        }
    },

      computed: {
        ...mapState({
            mdAccountSource: (state) => state.BASE.mdAccountSource || {},
        }),
    },

    methods: {

        handleSelectInstrumentId(item) {
            const { instrument_id, exchange_id } = item;
            this.$set(this.tickerForm, 'instrumentId', instrument_id);
            this.$set(this.tickerForm, 'exchangeId', (exchange_id || "").toUpperCase());

            this.$nextTick().then(() => {
                this.$refs["tickerForm"].validateField('instrumentId');
            });
        },
        
        handleClose () {
            this.$emit('update:visible', false);
        },

        handleSubmitSetting () {
            this.$refs["tickerForm"].validate(valid => {
                if (valid) {
                    this.$emit('confirm', deepClone(this.tickerForm))
                    this.handleClose()
                }
            })
        }
    }

}
</script>

<style>

</style>