<template>
	<el-form
		ref="extForm"
		:label-width="labelWidth"
		:model="form"
		class="ext-form"
	>
		<el-form-item 
            v-for="item of configList" :key="item.key"
            :label="item.name"
            :prop="item.key"
            :rules="buildValidators(item)"
			:class="{ 'is-radio': item.type === 'radio' }"
            >
                <el-col :span="19">
                    <el-input size="mini" :disabled="isDisabled(item.key)" :class="item.key" v-if="item.type === 'str'" v-model="form[item.key]" :type="item.key" ></el-input>
                    <el-input size="mini" :disabled="isDisabled(item.key)" :class="item.key" v-if="item.type === 'password'" :type="item.key" v-model="form[item.key]"  show-password></el-input>
                    <el-switch size="mini" :disabled="isDisabled(item.key)" :class="item.key" v-if="item.type === 'bool'" :value="form[item.key]" @change="e => handleInputValue(item.key, e)"></el-switch>
                    <el-input-number size="mini" :disabled="isDisabled(item.key)" :class="item.key" v-if="item.type === 'int'" :controls="false" :precision="0"  v-model="form[item.key]"></el-input-number>
                    <el-input-number size="mini" :disabled="isDisabled(item.key)" :class="item.key" v-if="item.type === 'float'" :controls="false" v-model="form[item.key]"></el-input-number>
                    <span class="account-setting-path path-selection-in-dialog text-overflow" v-if="item.type === 'file'" :title="form[item.key]">{{form[item.key]}}</span>                    
                    <el-button size="mini" icon="el-icon-more" v-if="item.type === 'file'" @click="handleSelectFile(item.key)"></el-button>
                    <el-select :disabled="isDisabled(item.key)" :class="item.key" size="mini" v-if="item.type === 'select'" :multiple="item.multiple" collapse-tags v-model="form[item.key]">
                        <el-option
						v-for="option in resolvedSelectOrRatioOptions(item)"
						:key="option.value"
						:label="option.name"
						:value="option.value">
                        </el-option>
                    </el-select>
					 <el-radio-group :disabled="isDisabled(item.key)" :class="item.key" size="mini" v-if="item.type === 'radio'" v-model="form[item.key]">
                        <el-radio 
						v-for="option in resolvedSelectOrRatioOptions(item)"
						:key="+option.value"
						:label="+option.value">
							{{ option.name }}
						</el-radio>
                    </el-radio-group>
					<el-time-picker size="mini" :class="item.key" v-if="item.type === 'timePicker'" v-model="form[item.key]" @blur="handleChangeTimePicker(item.key)" :clearable="true" :picker-options="{ selectableRange: '00:00:00 - 23:59:59', format: 'HHmmss' }">
					</el-time-picker>
					<el-select :disabled="isDisabled(item.key)" :class="item.key" size="mini" v-if="item.type === 'account'"  v-model.trim="form[item.key]">
                        <el-option
                            v-for="account in tdList"
                            :key="account.account_id.toAccountId()"
                            :label="account.account_id.toAccountId()"
                            :value="account.account_id">
                            <span style="color: #fff">{{account.account_id.toAccountId()}}</span>
                            <el-tag :type="getAccountType(account.source_name).type">{{(SourceTypeConfig[getAccountType(account.source_name).typeName] || {}).name || ''}}</el-tag>
                            <span style="float: right">可用：{{getAvailCash(account.account_id)}}</span>
                        </el-option>
                    </el-select>
					<el-select :disabled="isDisabled(item.key)" :class="item.key" size="mini" v-if="item.type === 'source'"  v-model.trim="form[item.key]">
						<el-option
						v-for="source in Object.keys(mdAccountSource)"
						:key="source"
						:label="source"
						:value="source"
						>
						<span>{{ source }}</span>
						<el-tag :type="mdAccountSource[source].type">{{(SourceTypeConfig[mdAccountSource[source].typeName] || {}).name || ''}}</el-tag>
						</el-option>	
					</el-select>
					 <el-select :disabled="isDisabled(item.key)" :class="item.key" size="mini" v-if="item.type === 'exchangeId'" v-model.trim="form[item.key]">
                        <el-option
						v-for="option in ExchangeIds"
						:key="option.value"
						:label="option.name"
						:value="option.value">
                        </el-option>
                    </el-select>
					<el-autocomplete 
					:disabled="isDisabled(item.key)" 
					:class="item.key" 
					size="mini"
                    ref="insturment-id-input"
					v-if="item.type === 'instrumentId'"
                    v-model="form[item.key]"
                    :fetch-suggestions="querySearch"
                    @select="e => handleSelectInstrumentId(item.key, item.exchangeIdKey, e)"
                    >
                        <template v-slot="{ item }">
							<div class="make-order-instrument-ids__warp">
                                <div class="make-order-instrument-id-item">
                                    <span class="ticker">{{ item.instrument_id }}</span>
                                    <span class="name">{{ item.instrument_name }}</span>
                                </div>
                                <div class="make-order-instrument-id-item">{{ (item.exchange_id || '').toUpperCase() }}</div>
                            </div>
                        </template>
                    </el-autocomplete>
                </el-col>
                <el-col :span="2" :offset="1" v-if="item.tip">
                    <el-tooltip class="item" effect="dark" :content="item.tip" placement="right">
                        <i class="el-icon-question mouse-over"></i>
                    </el-tooltip>
                </el-col>
            </el-form-item>
	</el-form>
</template>

<script>
import Vue from 'vue';
import moment from 'moment';
import { mapState } from 'vuex';
import { Autocomplete } from 'element-ui';

import { deepClone } from '__gUtils/busiUtils';
import { OffsetName, SideName, SourceTypeConfig, ExchangeIds } from 'kungfu-shared/config/tradingConfig';

import instrumentsMixin from '@/assets/js/mixins/instrumentsMixin';

Vue.use(Autocomplete)

export default {
	mixins: [ instrumentsMixin ],

	props: {

		configList: {
			type: Array,
			default: () => []
		},

		//确认唯一
		uniKeyValidator: {
			type: Function,
			default: (rule, value, callback) => { callback() }
		},

		labelWidth: {
			type: String,
			default: "80px"
		},

		uniKey: {
			type: [ String, Array ],
			default: () => ""
		},

		//添加/删除
		value: {
			type: Object,
			default: () => ({})
		},
		method: {
			type: String,
			default: 'add'
		},
	},

	data () {

		this.ExchangeIds = Object.keys(ExchangeIds || {}).map(key => {
			return {
				value: key,
				name: ExchangeIds[key]
			}
		})

        this.SourceTypeConfig = SourceTypeConfig;
		this.kungfuKeywordsData = {
			offset: OffsetName,
			side: SideName
		}

		return {
			form: deepClone(this.value || {})
		}
	},

	mounted () {
		this.initForm();
	},

	computed: {
		...mapState({
			tdList: state => state.ACCOUNT.tdList || [], 
            tdAccountSource: state => state.BASE.tdAccountSource || {},
			mdAccountSource: state => state.BASE.mdAccountSource || {},
			accountsAsset: state => state.ACCOUNT.accountsAsset,

		}),
	},

	watch: {
		form: {
			deep: true,
			handler (newVal) {
				this.$emit('input', newVal)
			}
		}, 

		mdAccountSource (val) {
			console.log(val, '===')
		}
	},

	methods: {		
		handleSelectInstrumentId (key, exchangeIdKey, item) {
			exchangeIdKey = exchangeIdKey || 'exchangeId';
            const { instrument_id, exchange_id } = item;
            this.$set(this.form, key, instrument_id)
            this.$set(this.form, exchangeIdKey, (exchange_id || '').toUpperCase())
            
            this.$nextTick()
                .then(() => {
                    this.$refs['extForm'].validateField(key);
                })
            
        },

		//日期必须要重写，不然有问题
		handleChangeTimePicker (key) {
			const theTime = this.form[key];
			let mt = moment(theTime);
			const date = mt.format('YYYYMMDD');
			const today = moment().format('YYYYMMDD');
			const yearToday = moment().get('year');
			const monthToday = moment().get('month');
			const dayToday = moment().get('date');

			if (date !== today) {
				mt.set('year', yearToday)
				mt.set('month', monthToday)
				mt.set('date', dayToday)
			}

			this.$set(this.form, key, mt.valueOf())
		},

        handleInputValue (key, e) {
            this.$set(this.form, key, e)
        },

        //添加文件
        handleSelectFile(targetKey) {
            const dialog = remote.dialog;
            dialog.showOpenDialog({
                properties: ['openFile']
            }, (filePath) => {
                if(!filePath || !filePath[0]) return;
                this.$set(this.postForm, targetKey, filePath[0]);
                this.$refs.extForm.validateField(targetKey) //手动进行再次验证，因数据放在span中，改变数据后无法触发验证
            })
		},

		querySearch (queryString, cb) {
            const results = this.getSearchTickers(queryString);
            cb(results)
        },

		getSearchTickers (queryString = '') {
           return this.instrumentIds.filter(item => {
                const { instrument_id, instrument_name, exchange_id } = {
					instrument_id: '',
					instrument_name: '',
                    exchange_id: '',
                    ...item
                }

                if (`${instrument_id}${instrument_name}${exchange_id}`.toLowerCase().includes(queryString.toLowerCase())) return true;
                return false
            })
        },

		isDisabled (key) {
			return this.method === 'update' && this.isUniKey(key)
		},

		isUniKey (key) {
			if (typeof this.uniKey === 'string') {
				return this.uniKey === key
			} else {
				return this.uniKey.includes(key)
			}
		},
		
		//for paComponents
		validate (cb) {
			this.$refs.extForm.validate(cb)
		},
		
		initForm () {
  			this.configList.forEach(item => {
                const key = item.key;
                const type = item.type;
                const defaultVal = item.default;
                if((this.form[key] === undefined) || (this.form[key] === '')) {
                    if (type === 'int') {
                        if (!+defaultVal) {  
                            return
                        }
                    }
                    
                    if(item.default !== undefined) {
                        this.$set(this.form, key, item.default)
                    } else {
                        this.$set(this.form, key, '')
                    }
                }
			})
		},

		resolvedSelectOrRatioOptions (item) {
			const options = item.options || item.data || [];
			if (typeof options !== 'string' || !this.kungfuKeywordsData[options]) return options;
			const target = this.kungfuKeywordsData[options];
			return Object.keys(target || {})
				.map(key => {
					return {
						name: target[key],
						value: key
					}
				})			
		},

 		buildValidators(item) {
			let validators = [];
			
            if(this.uniKey === item.key && this.method === 'add'){
                validators.push({ validator: this.uniKeyValidator, trigger: 'blur' })
			}
			
            if(item.required) {
				validators.push({required: true, message: item.errMsg || `请填写${item.name}`, trigger: 'blur'})
			}

            if(item.validator && item.validator.length) {
                validators = [...validators, ...item.validator.map(v => ({validator: v, trigger: 'blur'}))]
            }
            
            return validators
		},
		
		 getAvailCash (accountId) {
            if(!accountId) return 0;
            const targetAccount = this.accountsAsset[accountId] || null
            if(!targetAccount) return 0
            return targetAccount.avail || 0
		},
		
		getAccountType (sourceName) {
            return this.tdAccountSource[sourceName] || {}
        },
	}
}
</script>

<style lang="scss">
        
        .el-form {

			&.ext-form {

				.el-form-item {
					margin-bottom: 10px;

					&:last-child {
						margin-bottom: 10px;
					}

					&.is-radio {
						margin-bottom: 0;
					}

					.el-radio {
						margin-right: 18px;
					}

					.el-radio__label {
						font-size: 12px;
						padding-left: 5px;
					}
				}
			}
        }
    
</style>