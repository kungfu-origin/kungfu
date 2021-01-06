<template>
	<el-form
		ref="extForm"
		label-width="140px"
		:model="form"
	>
		<el-form-item 
            v-for="item of configList" :key="item.key"
            :label="item.name"
            :prop="item.key"
            :rules="buildValidators(item)"
            >
                <el-col :span="19">
                    <el-input size="mini" :class="item.key" v-if="item.type === 'str'" :value="form[item.key]" @input="e => handleInputValue(item.key, e)" :type="item.key" :disabled="method == 'update' && (uniKey == item.key)"></el-input>
                    <el-input size="mini" :class="item.key" v-if="item.type === 'password'" :type="item.key" :value="form[item.key]" @input="e => handleInputValue(item.key, e)" :disabled="method == 'update' && (uniKey == item.key)" show-password></el-input>
                    <el-switch size="mini" :class="item.key" v-if="item.type === 'bool'" :value="form[item.key]" @change="e => handleInputValue(item.key, e)"></el-switch>
                    <el-input-number size="mini" :class="item.key" v-if="item.type === 'int'" :controls="false" :value="form[item.key]" @input="e => handleInputValue(item.key, e)"></el-input-number>
                    <el-input-number size="mini" :class="item.key" v-if="item.type === 'float'" :controls="false" :value="form[item.key]" @input="e => handleInputValue(item.key, e)"></el-input-number>
                    <span class="account-setting-path path-selection-in-dialog text-overflow" v-if="item.type === 'file'" :title="form[item.key]">{{form[item.key]}}</span>                    
                    <el-button size="mini" icon="el-icon-more" v-if="item.type === 'file'" @click="handleSelectFile(item.key)"></el-button>
                    <el-select :class="item.key" size="mini" v-if="item.type === 'select'" :multiple="item.multiple" collapse-tags :value="form[item.key]" @change="e => handleInputValue(item.key, e)" placeholder="请选择">
                        <el-option
						v-for="option in item.data || item.options"
						:key="option.value"
						:label="option.name"
						:value="option.value">
                        </el-option>
                    </el-select>
					 <el-radio-group :class="item.key" size="mini" v-if="item.type === 'radio'" :value="form[item.key]" @change="e => handleInputValue(item.key, e)">
                        <el-radio 
						v-for="option in item.data || item.options"
						:key="option.value"
						size="mini"  
						label="option.value">
							{{ option.name }}
						</el-radio>
                    </el-radio-group>

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

import { deepClone } from '__gUtils/busiUtils';

export default {
	props: {

		configList: {
			type: Array,
			default: () => []
		},

		//确认唯一
		uniKeyValidator: {
			type: Function,
			default: () => {}
		},
		uniKey: "",

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

		return {
			form: deepClone(this.value || {})
		}
	},

	mounted () {
		this.initForm();
	},

	watch: {
		form: {
			deep: true,
			handler (newVal) {
				this.$emit('input', newVal)
			}
		} 
	},

	methods: {	

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
                    
                    if(item.default) {
                        this.$set(this.form, key, item.default)
                    } else {
                        this.$set(this.form, key, '')
                    }
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
		
        
	}


}
</script>

<style>

</style>