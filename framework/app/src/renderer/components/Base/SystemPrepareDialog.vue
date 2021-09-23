<template>
    <el-dialog
        :title="title"
        class="system-prepare-dialog"
        :visible="visible"
        :show-close="false"
        :close-on-click-modal="false"
        width="450px"
        >
            <div style="margin: 10px 0 20px" v-for="item in status" :key="item.key">
                <tr-status :value="item.status ? '100' : '3'" :hasText="false"></tr-status>
                {{ item.status  ? tips[item.key].done : tips[item.key].loading }}
            </div>
    </el-dialog>
</template>

<script>
export default {

    props: {
        
        title: String,

        visible: Boolean,

        status: Array, // [{ key: string, status: boolean }]

        tips: Object //{ [key]: { "done": "xxx", "loading": "xxxx" } }
    },
}
</script>

<style lang="scss">

 .system-prepare-dialog {
    .tr-dot-content {
        margin-right: 5px;
    }
 }

</style>