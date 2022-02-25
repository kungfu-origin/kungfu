<script setup lang="ts">
import { message } from 'ant-design-vue';
import { storeToRefs } from 'pinia';
import { computed, getCurrentInstance, onMounted, ref } from 'vue';
import { useModalVisible } from '../../assets/methods/uiUtils';
import { useGlobalStore } from '../../pages/index/store/global';

const props = withDefaults(
  defineProps<{
    visible: boolean;
    targetBoardId: number;
  }>(),
  {
    visible: false,
    targetBoardId: -1,
  },
);

const app = getCurrentInstance();
const { modalVisible, closeModal } = useModalVisible(props.visible);
const availKfBoards = ref<string[]>([]);
const selectedBoard = ref<string>('');
const { boardsMap } = storeToRefs(useGlobalStore());
const addedBoards = computed(() => {
  return boardsMap.value[props.targetBoardId]?.contents || [];
});

onMounted(() => {
  if (app?.proxy) {
    availKfBoards.value = app.proxy.$availKfBoards || [];
    if (availKfBoards.value.length) {
      const availKfBoardsResolved = availKfBoards.value.filter(
        (item) => !addedBoards.value.includes(item),
      );
      if (availKfBoardsResolved.length) {
        selectedBoard.value = availKfBoardsResolved[0];
      } else {
        selectedBoard.value = '';
      }
    }
  }
});

function handleComfirm() {
  if (!selectedBoard.value) {
    message.error('请选择要添加的面板');
    return;
  }

  if (
    props.targetBoardId === -1 ||
    !boardsMap.value[props.targetBoardId] ||
    boardsMap.value[props.targetBoardId]?.contents === undefined
  ) {
    message.error('添加面板目标错误');
    return;
  }

  useGlobalStore()
    .addBoardByContentId(props.targetBoardId, selectedBoard.value)
    .then(() => {
      message.success('操作成功');
      closeModal();
    })
    .catch(() => {
      message.error('操作失败');
    });
}
</script>
<template>
  <a-modal
    :width="520"
    class="kf-add-board-modal"
    v-model:visible="modalVisible"
    title="添加面板"
    :destroyOnClose="true"
    @cancel="closeModal"
    @ok="handleComfirm"
  >
    <div class="kf-add-board__content">
      <a-radio-group v-model:value="selectedBoard">
        <a-radio-button
          v-for="item in availKfBoards"
          :value="item"
          :disabled="addedBoards.includes(item)"
        >
          {{ item }}
        </a-radio-button>
      </a-radio-group>
    </div>
  </a-modal>
</template>
<style lang="less">
.kf-add-board__content {
  .ant-radio-group {
    display: flex;
    justify-content: flex-start;
    flex-wrap: wrap;

    .ant-radio-button-wrapper {
      margin-top: 10px;
      margin-left: 10px;
    }
  }
}
</style>
