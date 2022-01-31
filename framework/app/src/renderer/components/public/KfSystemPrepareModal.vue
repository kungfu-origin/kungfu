<script setup lang="ts">
interface Status {
  key: string;
  status: 'done' | 'loading' | undefined;
}

interface Txt {
  [prop: string]: {
    done: string;
    loading: string;
  };
}

defineProps<{
  visible: boolean;
  status: Status[];
  txt: Txt;
  title: string;
}>();
</script>
<template>
  <a-modal
    :width="400"
    class="kf-system-prepare-modal"
    :title="title"
    :visible="visible"
    :closable="false"
    :maskClosable="false"
    :footer="null"
  >
    <div class="prepare-item" v-for="item in status" :key="item.key">
      <span
        :class="[
          'kf-dot',
          item.status === 'done' ? 'kf-color-running' : 'kf-color-waiting',
          item.status === 'loading' ? 'kf-dot-wave' : '',
        ]"
      ></span>
      <span>{{ item.status ? txt[item.key][item.status] : '' }}</span>
    </div>
  </a-modal>
</template>
<style lang="less">
.kf-system-prepare-modal {
  .prepare-item {
    margin: 10px 10px 10px 0;
    span {
      margin-right: 10px;
    }
  }
}
</style>
