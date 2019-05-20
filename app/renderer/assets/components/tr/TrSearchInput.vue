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

    methods: {
        clearData(){
            const t = this;
            t.active = false;
			t.searchKeyword = '' 
			t.$emit('input', '')
		},

		handleActive(){
			const t = this;
			t.active = true;
			t.$nextTick().then(() => {
				const $searchInput = t.$refs['search-input'].$el;
				$searchInput.querySelectorAll('input').forEach($input => $input.focus())
			})
		},
		
		handleInput(val){
			const t = this;
			t.$emit('input', val.trim())
		},

		handleBlur(){
			const t = this;
			if(!t.value) t.clearData();
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