<template>
    <div class="tr-search-input">
		<i v-if="!active" class="el-icon-search mouse-over" title="搜索" @click="handleActive"></i>
		<el-input 
		ref="search-input"
		v-if="active"
		size="mini" 
		placeholder="关键字" 
		prefix-icon="el-icon-search" 
		v-model.trim="searchKeyword"
		@input="handleInput"
		@blur.stop="handleBlur"
		>
			<i slot="suffix" class="el-input__icon el-icon-close mouse-over" @click.stop="clearData"></i>
		</el-input>
    </div>
</template>
<script>
export default {
    name: 'tr-search-input',
    props: {
        value: {
            type: [Number, String],
            default: ''
        }
    },

    data(){
        return {
            active: false,
            searchKeyword: ''
        }
    },

	watch: {

		value (newVal, oldVal) {
			if ( !newVal || !newVal.trim()) {
				if (oldVal.trim() !== '') {
					this.clearData();
				}
			}
		}
	},

    methods: {
        clearData(){
            this.active = false;
			this.searchKeyword = '' 
			this.$emit('input', '')
		},

		handleActive(){
			this.active = true;
			this.$nextTick().then(() => {
				const $searchInput = this.$refs['search-input'].$el;
				$searchInput.querySelectorAll('input').forEach($input => $input.focus())
			})
		},
		
		handleInput(val){
			this.$emit('input', val.trim())
		},

		handleBlur(){
			if(!this.value) this.clearData();
		}
    }
}
</script>
<style lang="scss">
	.tr-search-input{
		.el-input{
			width: 101px;
		}
	}
</style>