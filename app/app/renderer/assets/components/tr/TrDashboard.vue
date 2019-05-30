<template>
    <div class="tr-dashboard" :style="{'height': height, 'min-height': minHeight}">
        <div class="tr-dashboard-warp">
            <div class="tr-dashboard-header" v-if="!noHeader">
                <div class="tr-dashboard-title">{{title}}</div>
                <slot name="dashboard-header"></slot>
            </div>
            <div :class="{'no-header': noHeader, 'tr-dashboard-body': true}">
                <slot></slot>
            </div>
        </div>
       
    </div>
</template>
<script>
export default {
    name: 'tr-dashboard',
    props: {
        title: {
            type: String,
            default: ''
        },

        noHeader: {
            type: Boolean,
            default: false
        },

        height: {
            type: String,
            default: '100%'
        },
        minHeight: {
            type: String,
            default: '0px'
        }
        
    }
    
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";

.tr-dashboard{
    width: 100%;
    height: 100%;
    padding-right: 8px;
    padding-bottom: 8px;
    box-sizing: border-box;
}

.tr-dashboard-warp{
    height: 100%;
    background: $bg_card;   
}

.tr-dashboard-body{
    position: relative;
    height: calc(100% - 36px);
    overflow: hidden;
    overflow-y: auto;
}
.tr-dashboard-body.no-header{
    height: 100%;
}
.tr-dashboard-body .table-body{
    width: 100%;
}



.tr-dashboard-header{
    height: 36px;
    line-height: 36px;
    padding: 0px 5px 0px 10px;
    overflow: hidden;
    >div{
        float: right;
    }
    .tr-dashboard-title{
        display: inline-block;
        float: none;
    }
    .header-item{
        display: inline-block;
        height: 36px;
        line-height: 36px;
        padding-left: 8px;
        .el-button{
            border: none;
            i{
                color: $font_1;
            }
        }
        .el-date-editor{
            vertical-align: middle;
        }
        i.fa{
            font-size: 12px;
            padding-right: 5px;
        }
    }
    
}
</style>

