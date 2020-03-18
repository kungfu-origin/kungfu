<template>
<el-dialog
	custom-class="global-setting-dialog"
	width="90%"
	title="全局设置"
	top="5vh"
	:visible="visible"
	:close-on-click-modal="false"
	:close-on-press-escape="true"
	@close="handleCancel"
>
	<div class="global-setting-content">
	<div class="dir-list">
		<el-collapse v-model="activeSettingTypes">
		<el-collapse-item
			:title="item.name"
			:name="item.key"
			v-for="item in settingConfig"
			:key="item.key"
		>
			<ul>
			<li
				v-for="set in item.config"
				:key="set.key"
				:class="{'active': activeSettingItem === `${item.key}-${set.key}`}"
				@click="handleClickSettingType(item.key, set.key)"
			>{{set.name}}</li>
			</ul>
		</el-collapse-item>
		</el-collapse>
	</div>
	<div class="setting-content">
		<div class="setting-type-item" v-for="setting in settingConfig" :key="setting.key">
		<p class="setting-type-item__header">{{setting.name}}</p>
		<ul class="setting-type-item__content">
			<li
			class="setting-item global-setting-item"
			v-for="item in Object.values(setting.config || {})"
			:key="item.key"
			:id="`setting-item-${setting.key}-${item.key}`"
			>
			<p class="setting-item__header">{{item.name}}</p>

			<div class="setting-sub-item__item" v-for="config in item.config" :key="config.key">
				<p class="setting-sub-item__header">{{config.name}}</p>
				<p class="setting-sub-item__tip">{{config.tip}}</p>
				<!-- 普通类型 -->
				<div
				class="setting-sub-item__input-content"
				v-if="settingConfig[setting.key].value && (config.type !== 'table')"
				>
					<el-input
						:class="`${item.key}_${config.key}`"
						v-if="config.type === 'str'"
						:type="item.key"
						v-model="settingConfig[setting.key].value[item.key][config.key]"
						@blur="handleIuput(setting.key)"
					></el-input>
					
					<el-switch
						:class="`${item.key}_${config.key}`"
						v-if="config.type === 'process'"
						:value="processStatus[config.target] === 'online'"
						@change="e => handleSwitchProcess(e, config, settingConfig[setting.key].value[item.key])"
					></el-switch>

					<span
						class="tr-oper"
						v-if="config.type === 'process'"
						@click.stop="handleOpenLogFile(config)"
					>
						<i class="el-icon-document mouse-over"></i> {{item.name}}日志
					</span>
					
					<el-switch 
					:class="`${item.key}_${config.key}`" 
					v-if="config.type === 'bool'" 
					v-model.trim="settingConfig[setting.key].value[item.key][config.key]"
					@change="handleIuput(setting.key)"
					size="small"
					></el-switch>

					<span
						class="tr-oper"
						v-if="(config.type === 'bool') && (item.key === 'strategy') && (config.key === 'python')"
						@click.stop="handleOpenWhlFolder()"
					>
						<i class="el-icon-folder mouse-over"></i> 打开whl文件夹
					</span>

					<el-select
						:class="`${item.key}_${config.key}`"
						v-if="config.type === 'select'"
						:multiple="config.multiple"
						collapse-tags
						v-model="settingConfig[setting.key].value[item.key][config.key]"
						@change="handleIuput(setting.key)"
						size="small"
					>
					<el-option
						v-for="option in config.data"
						:key="option.value"
						:label="option.name"
						:value="option.value"
						></el-option>
					</el-select>
					
					<el-select
						:class="`${item.key}_${config.key}`"
						v-if="config.type === 'sources'"
						:multiple="config.multiple"
						collapse-tags
						v-model="settingConfig[setting.key].value[item.key][config.key]"
						@change="handleIuput(setting.key)"
						size="small"
					>
						<el-option
						v-for="option in sourceList"
						:key="option.value"
						:label="option.name"
						:value="option.value"
						></el-option>
					</el-select>
				</div>

				<!-- table类型 -->
				<div class="setting-sub-item__table-content" v-else>
					<ul class="table-content">
						<li class="table-header">
							<el-col :span="cell.cellWidth || 3" v-for="cell in config.row" :key="cell.unique_key">
								<div class="cell-name">{{cell.name}}</div>
							</el-col>
						</li>
						<div class="table-body">
						<li
							class="table-rows"
							v-for="(row, index) in tables[config.target]"
							:key="row.rowid"
						>
							<el-col :span="cell.cellWidth || 3" v-for="cell in config.row" :key="cell.unique_key">
							<el-input
								size="mini"
								:class="cell.key"
								v-if="cell.type === 'str'"
								v-model="row[cell.key]"
							></el-input>
							<el-input-number
								size="mini"
								:class="cell.key"
								v-if="cell.type === 'int'"
								:controls="false"
								v-model="row[cell.key]"
							></el-input-number>
							<el-input-number
								size="mini"
								:class="cell.key"
								v-if="cell.type === 'float'"
								:controls="false"
								v-model="row[cell.key]"
							></el-input-number>
							<el-select
								:class="cell.key"
								v-if="cell.type === 'select'"
								:multiple="cell.multiple"
								collapse-tags
								v-model="row[cell.key]"
								size="mini"
							>
								<el-option
								v-for="option in cell.data"
								:key="option.value"
								:label="option.name"
								:value="+option.value"
								></el-option>
							</el-select>
							</el-col>
							<el-col :span="1">
							<i
								class="el-icon-circle-plus mouse-over"
								@click="handleAddRow(config.target, config.row, index)"
							></i>
							</el-col>
							<el-col :span="1"  v-if="tables[config.target].length > 1">
							<i
								class="el-icon-remove mouse-over"
								@click="handleRemoveRow(config.target, index)"
							></i>
							</el-col>
						</li>
						</div>
					</ul>
				</div>
			</div>
			</li>
		</ul>
		</div>
	</div>
	</div>
</el-dialog>
</template>

<script>
import Vue from "vue";
import { mapGetters, mapState } from "vuex";
import { Collapse, CollapseItem } from "element-ui";
import { readJsonSync, outputJsonSync } from "__gUtils/fileUtils";
import { LOG_DIR, KUNGFU_RESOURCES_DIR } from "__gConfig/pathConfig";
import { getExtensionConfigs, getExtensions, getSourceList, debounce, throttle } from "__gUtils/busiUtils";
import { buildSystemConfig } from "__gConfig/systemConfig";
import { switchCustomProcess } from "__io/actions/base";
import { getKfCommission, setKfCommission } from '__gUtils/kungfuUtils';

const { shell }  = require('electron').remote 
const path = require("path");

Vue.use(Collapse);
Vue.use(CollapseItem);

export default {
name: "global-setting-dialog",
props: {
	visible: {
	type: Boolean,
	default: false
	}
},

data() {
	return {
		activeSettingTypes: ["system", "trading"],
		activeSettingItem: "",
		settingConfig: buildSystemConfig(),

		sourceList: [],
		tables: {
			commission: []
		},
		tablesSaveMethods: {
			commission: {
				filters: ["instrument_id", "mode", "exchange_id"], //必填，且唯一
				method: setKfCommission,
			}
		}
	};
},

async beforeMount() {
	const t = this;
	t.getSourceListOptions();

	//获取
	t.tables.commission = await getKfCommission();
},

mounted() {
	const t = this;
	//设置高亮
	const firstKeyOfSystemConfig = Object.keys(t.settingConfig.system.config || {})[0] || ''
	t.activeSettingItem = `system-${firstKeyOfSystemConfig}`

	//滚动
	t.$nextTick()
	.then(() => {
		const $settingContent = document.querySelectorAll('.setting-content')[0]
		$settingContent.addEventListener('scroll', throttle(t.setActiveMenu))	
	})

},

computed: {
	...mapState({
		processStatus: state => state.BASE.processStatus || {}
	})
},

beforeDestroy() {
	const t = this;
	t.saveTables();
},

methods: {
	handleClickSettingType(typeKey, itemKey) {
		const t = this;
		t.activeSettingItem = `${typeKey}-${itemKey}`;
		document
			.querySelector(`#setting-item-${t.activeSettingItem}`)
			.scrollIntoView();
	},

	handleCancel() {
		const t = this;
		t.close();
	},

	handleIuput(settingKey) {
		const t = this;
		const settingData = t.settingConfig[settingKey].value;
		const outputPath = t.settingConfig[settingKey].outputPath;
		t.$nextTick()
			.then(() => outputJsonSync(outputPath, settingData || {}))
			.then(() => readJsonSync(outputPath))
			.then(config => {
			if (!config) return;
			t.$set(t.settingConfig[settingKey], "value", config);
			});
	},

	handleSwitchProcess(value, config, settingData) {
		const t = this;
		//开启
		if (value) {
			switchCustomProcess(value, config.target);
		} else {
			switchCustomProcess(value, config.target);
		}
	},

	//打开日志
	handleOpenLogFile(config) {
		const logPath = path.join(LOG_DIR, `${config.target}.log`);
		this.$showLog(logPath);
	},

	//打开文件夹 
	handleOpenWhlFolder() {
		shell.showItemInFolder(path.join(KUNGFU_RESOURCES_DIR, 'python'));
	},

	//table 添加row
	handleAddRow(target, row, index) {
		const t = this;
		const tmp = [{}, ...row].reduce((a, b) => {
			a[b.key] = b.default;
			return a;
		});
		t.tables[target].splice(index + 1, 0, {
			...tmp,
			rowid: `tmp_${+new Date().getTime()}`
		});
	},

	//table remove row
	handleRemoveRow(target, index) {
		const t = this;
		t.tables[target].splice(index, 1);
	},

	setActiveMenu() {
		const t = this;
		const $settingItems = Array().slice.call(document.querySelectorAll('.global-setting-item'));
		const visibleItems = $settingItems.filter(settingItem => {
			const visibleData = settingItem.getBoundingClientRect();
			const top = visibleData.top - 145;
			if(top > 0) return true;
			else return false;
		})
		if(visibleItems[0]) {
			const idVal = visibleItems[0].getAttribute('id')
			t.activeSettingItem = idVal.split('setting-item-')[1]
		}
	},

	getSourceListOptions() {
		const t = this;
		getSourceList().then(sourceList => (t.sourceList = sourceList));
	},

	saveTables() {
		const t = this;
		Object.keys(t.tablesSaveMethods || {}).forEach(key => {
			const filters = t.tablesSaveMethods[key].filters;
			const saveMethod = t.tablesSaveMethods[key].method;

			//去重
			const targetDataResolve = [{}, ...t.tables[key]].reduce((a, b) => {
				const rowKey = filters.map(key => b[key].toString() || "").join("_");
				a[rowKey] = b;
				return a;
			});

			//去掉无效的key
			const targetData = Object.values(targetDataResolve || {}).filter(row => {
				delete row.rowid;
				let i,
				len = filters.length;
				for (i = 0; i < len; i++) {
					const key = filters[i];
					if (row[key].toString() === "") return false;
				}
				return true;
			});
			
			saveMethod(Object.freeze(targetData))
				.catch(err => console.error(err));
		});
	},

	close() {
		this.$emit("update:visible", false);
	}
}
};
</script>

<style lang="scss">
@import "@/assets/scss/skin.scss";
.global-setting-dialog {
height: 88%;
.el-dialog__body {
	height: calc(100% - 55px);
	box-sizing: border-box;
}
.global-setting-content {
	height: 100%;
}
.dir-list {
	width: 200px;
	display: inline-block;
	vertical-align: top;
	.el-collapse-item__content {
	li {
		height: 30px;
		line-height: 30px;
		padding: 0 30px;
		background: $bg_card;
		cursor: pointer;
	}
	li.active {
		color: $vi;
	}
	li:hover {
		color: $font_5;
	}
	li:active {
		color: $vi;
	}
	}
}
.setting-content {
	display: inline-block;
	width: calc(100% - 300px);
	height: calc(100% - 55px);
	margin-left: 30px;
	padding-bottom: 150px;
	box-sizing: border-box;
	overflow: auto;

	.setting-type-item {
	padding-bottom: 30px;
	}

	.setting-type-item__header {
	font-size: 20px;
	font-weight: bold;
	color: $font;
	padding-bottom: 20px;
	}
	.setting-item__header {
	font-size: 18px;
	color: $font_5;
	padding-bottom: 15px;
	}
	.setting-sub-item__item {
	.setting-sub-item__header {
		color: $vi;
		font-size: 16px;
		padding-bottom: 10px;
	}
	.setting-sub-item__tip {
		font-size: 14px;
		padding-bottom: 10px;
	}
	.setting-sub-item__input-content {
		width: 300px;
		padding-bottom: 30px;
	}
	}

	.tr-oper {
	padding-left: 20px;
	vertical-align: middle;
	i {
		font-size: 14px !important;
	}
	}
}
}
.setting-sub-item__table-content {
.table-content {
	width: calc(100% - 10px);
}
.table-header {
	height: 35px;
	.el-col {
		font-size: 12px;
		padding: 8px 8px 0px 0px;
		box-sizing: border-box;
	}
}
.table-body {
	width: 100%;
	max-height: 400px;
	overflow-y: auto;
}
.table-rows {
	height: 45px;
	padding-bottom: 4px;
	box-sizing: border-box;


	.el-col {
	padding: 8px 8px 0px 0px;
	box-sizing: border-box;
	i {
		cursor: pointer;
		vertical-align: sub;
	}
	}
	.cell-name {
	font-size: 12px;
	padding-bottom: 3px;
	box-sizing: border-box;
	}
}
}
</style>