<template>
    <div class="kf-test__warp">111</div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $registedKfUIComponents: string[];
    }
}

export default defineComponent({
    name: '交易账户',
    mounted() {
        console.log(22211);
        console.log(333);

        console.log(this.$registedKfUIComponents);
    },
});
</script>

<style lang="ts"></style>
