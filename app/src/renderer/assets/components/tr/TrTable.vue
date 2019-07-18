<template>
    <div class="tr-table">
        <ul class="tr-table-header">
            <li 
            class="text-overflow tr-table-cell" 
            v-for="column in schema" 
            :key="column.prop" 
            :title="column.label"
            :style="{
                'width': column.width ? column.width : headerWidth[column.prop]
            }">
                {{column.label}}
            </li>
        </ul>
        <div class="tr-table-body" ref="tr-table-body" >
            <RecycleScroller
            ref="tr-scroller-table"
            v-if="scrollerType !== 'dynamic' && data && data.length "
            class="tr-table-scroller"
            :items="data"
            :item-size="20"
            key-field="id"
            :buffer="10"
            >
                <template v-slot="{item}">
                    <ul class="tr-table-row">
                        <li 
                        :title="item[column.prop] || ''"
                        :class="[
                            renderCellClass(column.prop, item),                        
                            'tr-table-cell', 
                            'text-overflow',
                            column.type === 'operation' ? 'oper' : ''
                        ]"
                        v-for="column in schema" 
                        :key="column.prop"       
                        :style="{                             
                                'max-width': headerWidth[column.prop] || column.width,
                        }">
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
            v-else-if="scrollerType === 'dynamic' && data && data.length "
            class="tr-table-scroller tr-table-dynamic-scroller"            
            :items="data"
            :min-item-size="500"
            key-field="id"
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
                                column.width ? 'text-overflow' : ''
                            ]"
                            :title="item[column.prop] || ''"
                            v-for="column in schema" 
                            :key="column.prop"       
                            :style="{                             
                                'max-width': headerWidth[column.prop] || column.width
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
            type: [Object, Array]
        },


        //  type: "",
        //  label: String
        //  prop: String,
        //  width: String
        //  flex: Number
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
        t.calcBodyWidth()
        let $scrollerTable = t.$refs['tr-scroller-table'] || t.$refs['tr-dynamic-scroller-table'];
        window.addEventListener('resize', debounce(e => {
            t.calcBodyWidth()
                if(!$scrollerTable) $scrollerTable = t.$refs['tr-scroller-table'] || t.$refs['tr-dynamic-scroller-table'];
                $scrollerTable && $scrollerTable.forceUpdate && $scrollerTable.forceUpdate()              
        }, 300))
    },

    methods: {
        triggerToBottom(){
            const t = this;
            t.$nextTick().then(() => {
                const $scrollerTable = t.$refs['tr-scroller-table'] || t.$refs['tr-dynamic-scroller-table'];
                $scrollerTable && $scrollerTable.$el && ($scrollerTable.$el.scrollTop = 1000000000)
            })
        },

        calcBodyWidth(){
            const t = this;
            const $target = t.$refs['tr-table-body']
            if(!$target) return;
            t.bodyWidth = $target.clientWidth;
        },  
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
        height: 25px;
        line-height: 25px;
        background: $tab_header;
        white-space: nowrap;
        .tr-table-cell{
            height: 25px;
            line-height: 25px;
            display: inline-block;
            color: $font;
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
        .oper{
            text-align: right;
            font-size: 14px;
        }
    }
    .tr-table-row:hover{
        background: $bg_light;
    }

     .tr-table-cell{
        height: 20px;
        line-height: 20px;
        padding: 0 10px;
        box-sizing: border-box;
        word-wrap: break-word;
        flex: 1;
        color: $font_5;
        font-size: 12px;
        font-family: Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;
        user-select: text;
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

    .tr-table-body .tr-table-dynamic-scroller{
        .tr-table-row{
            .tr-table-cell{
                height: auto;
                line-height: 20px;
            }
        }
    }

  

}
</style>
