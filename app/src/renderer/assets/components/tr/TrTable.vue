<template>
    <div class="tr-table">
        <ul class="tr-table-header tr-table-row">
            <li 
            :class="[
            'text-overflow', 
            'tr-table-cell',
            column.type === 'number' ? 'number' : '',
            column.type === 'account-strategy' ? 'account-strategy' : '',
            column.type === 'operation' ? 'oper' : '',
            column.align === 'center' ? 'text-center' : ''
            ]" 
            v-for="column in schema" 
            :key="column.prop" 
            :title="column.label"
            :style="{
                'max-width': getHeaderWidth(column)
            }">
                {{column.label}}
            </li>
        </ul>
        <div class="tr-table-body" ref="tr-table-body" >
            <RecycleScroller
            ref="tr-scroller-table"
            v-if="scrollerType !== 'dynamic' && show "
            class="tr-table-scroller"
            :items="data"
            :item-size="20"
            key-field="id"
            :buffer="100"
            >
                <template v-slot="{item}">
                    <ul class="tr-table-row" 
                    @dblclick="$emit('dbclickRow', item)"
                    @mousedown="e => handleMousedown(e, item)"
                    >
                        <li 
                        :title="item[column.prop] || ''"
                        :class="[
                            renderCellClass(column.prop, item),                        
                            'tr-table-cell', 
                            'text-overflow',
                            item.update ? 'update' : '',
                            column.type === 'number' ? 'number' : '',
                            column.type === 'account-strategy' ? 'account-strategy' : '',
                            column.type === 'operation' ? 'oper' : '',
                            column.align === 'center' ? 'text-center' : ''
                        ]"
                        v-for="column in schema" 
                        :key="`${column.prop}_${item.id}_${item[column.prop]}`"       
                        :style="{                             
                            'max-width': getHeaderWidth(column)
                        }"
                        @click.stop="e => $emit('clickCell', e, item, column)"
                        >
                            <template v-if="column.type !== 'operation'">
                                {{item[column.prop]}}
                            </template>
                            <template v-else-if="column.type === 'operation'">
                                <slot name="oper" :oper="item"></slot>
                            </template>
                        </li>
                    </ul>
                </template>
            </RecycleScroller>
            <DynamicScroller 
            ref="tr-dynamic-scroller-table"
            v-else-if="scrollerType === 'dynamic' && show "
            class="tr-table-scroller tr-table-dynamic-scroller"            
            :items="data"
            :min-item-size="50"
            key-field="id"
            :buffer="100"
            >
                <template v-slot="{ item, index, active }">
                    <DynamicScrollerItem
                    :item="item"
                    :active="active"
                    :data-index="index"
                    :size-dependencies="sizeDependencies.map(key => (item[key]))"
                    >
                        <ul class="tr-table-row">
                            <li 
                            :class="[
                                renderCellClass(column.prop, item),
                                'tr-table-cell', 
                                column.width ? 'text-overflow' : '',
                                column.align === 'center' ? 'text-center' : ''
                            ]"
                            :title="item[column.prop] || ''"
                            v-for="column in schema" 
                            :key="`${column.prop}`"       
                            :style="{                             
                                'max-width': getHeaderWidth(column)
                            }">
                                <template v-if="column.type !== 'operation'">
                                    {{item[column.prop]}}
                                </template>
                                <template v-else-if="column.type === 'operation'">
                                    <slot name="oper" v-bind:props="item"></slot>
                                </template>
                            </li>
                        </ul>
                    </DynamicScrollerItem>
                </template>
            </DynamicScroller>
            <tr-no-data v-else></tr-no-data>
        </div>
    </div>
</template>
<script>
import Vue from 'vue';
import VueVirtualScroller from 'vue-virtual-scroller'
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css'
import {debounce, sum} from '__gUtils/busiUtils';

Vue.use(VueVirtualScroller)
export default {
    name: 'tr-table',
    props: {
        // ''(default), 'dynamic'(row的高度自计算)
        scrollerType: {
            default: '',
            type: String,
        },

        //当type为dynamic，指定以哪几列作为计算标准
        sizeDependencies: {
            default: () => [],
            type: Array
        },

        //必须有id项
        data: {
            default: [],
            type: [ Object, Array ]
        },

        schema: {
            type: Array,
            default: []
        },

        renderCellClass: {
            type: Function,
            default: () => {
                return ''
            }
        }
    },

    data(){
        return {
            bodyWidth: 0,
            isScroll: false,
            show: false
        }
    },

    watch: {
        data(newVal) {
            const t = this;
            if (newVal.length !== 0) {
                t.$nextTick().then(() => t.show = true)
            } else {
                t.$nextTick().then(() => t.show = false)
            }
        }
    },

    computed: {
        headerWidth(){
            const t = this;
            let widths = []; //column use with
            let flexs = []; //column use flex

            t.schema.forEach(item => {
                if(item.width !== undefined){
                    widths.push(item);
                }else{
                    flexs.push(item)
                }
            })

            //先减去 固定宽度集合，再算flex unit
            const flexWidthUnits  = sum(flexs.map(item => item.flex || 1))
            const widthForFlex = t.bodyWidth - (widths.length ? t.$utils.sum(widths.map(item => parseFloat(item.width))) : 0)
            const unit = (widthForFlex - 16) / flexWidthUnits
            let headerWidthCollection = {};
            [...widths, ...flexs].forEach(item => {
                headerWidthCollection[item.prop] = item.width ? item.width : (unit * (item.flex || 1) + 'px');
            });
            return headerWidthCollection
        }
    },

    mounted(){
        const t = this;
        t.calcBodyWidthHeight()
        let $scrollerTable = t.$refs['tr-scroller-table'] || t.$refs['tr-dynamic-scroller-table'];
        window.addEventListener('resize', debounce(e => {
            t.calcBodyWidthHeight()
                if(!$scrollerTable) $scrollerTable = t.$refs['tr-scroller-table'] || t.$refs['tr-dynamic-scroller-table'];
                $scrollerTable && $scrollerTable.forceUpdate && $scrollerTable.forceUpdate()              
        }, 300))
    },

    methods: {

        handleMousedown (e, item) {
            if (e.button === 2) {
                this.$emit('rightClickRow', item)
            }
        },

        triggerToBottom() {
            const t = this;
            t.$nextTick().then(() => {
                const $scrollerTable = t.$refs['tr-scroller-table'] || t.$refs['tr-dynamic-scroller-table'];
                $scrollerTable && $scrollerTable.$el && ($scrollerTable.$el.scrollTop = 1000000000)
            })
        },

        calcBodyWidthHeight() {
            const $target = this.$refs['tr-table-body']
            if(!$target) return;
            this.bodyWidth = $target.clientWidth;            
        },

        getHeaderWidth (column) {
            const prop = column.prop;
            const headerWidthByCalc = this.headerWidth[column.prop];
            const columnWidth = column.width;

            if ((headerWidthByCalc <= 0) || !headerWidthByCalc) {
                return columnWidth
            } else {
                return headerWidthByCalc
            }
        }
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.tr-table{
    display: flex;
    flex-direction: column;
    height: 100%;
    position: relative;
    .tr-table-header{
        display: flex;
        flex-direction: row;
        justify-content: flex-start;
        height: 25px;
        line-height: 25px;
        background: $tab_header;
        white-space: nowrap;
        padding-right: 16px;
        box-sizing: border-box;

        .tr-table-cell{
            height: 25px;
            line-height: 25px;
            display: inline-block;
            color: $font;
        }
        .tr-table-cell.number{
            text-align: right;
        }
    }

    .tr-table-body{
        width: 100%;
        height: calc(100% - 25px);
        position: absolute;
        transform: translateZ(0);
        top: 25px;
        .tr-table-scroller{
            height: 100%;
            width: 100%;
        }
    }

    .tr-table-row{
        display: flex;
        flex-direction: row;
        justify-content: flex-start;
        cursor: pointer;

        .oper{
            text-align: left;
            font-size: 14px;
            min-width: 40px;
        }

        .account-strategy {
            max-width: 100px !important;
        }
    }

    .tr-table-row:hover{
        background: $bg_light;
    }

     .tr-table-cell{
        height: 20px;
        line-height: 20px;
        padding: 0 6px;
        box-sizing: border-box;
        word-wrap: break-word;
        flex: 1;
        color: $font_5;
        font-size: 12px;
        font-family: Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;
        user-select: text;
        text-align: left;
    }
    .tr-table-cell.number{
        text-align: right;
    }
    .tr-table-cell.red{
        color: $red;
    }
    .tr-table-cell.green{
        color: $green;
    }
    .tr-table-cell.yellow{
        color: $vi;
    }
    .tr-table-cell.gray{
        color: $font;
    }
     .tr-table-cell.blue{
        color: $blue;
    }
    .tr-table-cell.update{
        animation: nanoBlink 1.1s 1;
    }
    .tr-table-cell.update.number.red{
        animation: nanoRedBlink 1.1s 1;
    }
    .tr-table-cell.update.number.green{
        animation: nanoGreenBlink 1.1s 1;
    }


    .tr-table-body .tr-table-dynamic-scroller{
        .tr-table-row{
            .tr-table-cell{
                height: auto;
                line-height: 20px;
                word-break: break-all;
            }
        }
    }

  

}
</style>
