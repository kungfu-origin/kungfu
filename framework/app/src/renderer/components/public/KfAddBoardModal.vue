<script setup lang="ts">
import { storeToRefs } from 'pinia';
import { computed, getCurrentInstance, onMounted, ref } from 'vue';
import { messagePrompt, useModalVisible } from '../../assets/methods/uiUtils';
import { useGlobalStore } from '../../pages/index/store/global';
import VueI18n, { useLanguage } from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

const { success, error } = messagePrompt();
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
const { isLanguageKeyAvailable } = useLanguage();
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
    error(t('select_board'));
    return;
  }

  if (props.targetBoardId === 0) {
    if (!boardsMap.value[0]) return error(t('add_board_error'));

    useGlobalStore()
      .addBoardFromEmpty(selectedBoard.value)
      .then(() => {
        success();
        closeModal();
      })
      .catch(() => {
        error();
      });

    return;
  }

  if (
    props.targetBoardId === -1 ||
    !boardsMap.value[props.targetBoardId] ||
    boardsMap.value[props.targetBoardId]?.contents === undefined
  ) {
    error(t('add_board_error'));
    return;
  }

  useGlobalStore()
    .addBoardByContentId(props.targetBoardId, selectedBoard.value)
    .then(() => {
      success();
      closeModal();
    })
    .catch(() => {
      error();
    });
}
</script>
<template>
  <a-modal
    v-model:visible="modalVisible"
    :width="520"
    class="kf-add-board-modal"
    :title="$t('add_board')"
    :destroy-on-close="true"
    @cancel="closeModal"
    @ok="handleComfirm"
  >
    <div class="kf-add-board__content">
      <a-radio-group v-model:value="selectedBoard">
        <a-radio-button
          v-for="item in availKfBoards"
          :key="item"
          :value="item"
          :disabled="addedBoards.includes(item)"
        >
          {{ isLanguageKeyAvailable(item) ? $t(item) : item }}
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
