<template>
  <div
    v-if="beforeDelate"
    class="c-app-code-file-node"
    :class="{
      folder: type == 'folder',
      file: type !== 'folder',
    }"
  >
    <div @click.stop="handleClickFile(fileNode)">
      <div
        class="each-files"
        :class="{
          'root-file': fileNode.root,
          active: fileNode.filePath === currentFile.filePath,
        }"
        :style="{ 'padding-left': `${curCount * 16 + 5}px` }"
      >
        <div class="file-top">
          <img class="file-icon" :src="iconPath" v-if="iconPath" />
          <span
            class="file-name"
            :class="{
              'root-name': fileNode.root,
              'normal-name': entryFile.filePath !== fileNode.filePath,
            }"
            v-if="fileNode && !onEditing && fileNode.name"
          >
            {{ fileNode.name }}
          </span>
          <a-input
            v-else-if="onEditing"
            id="edit-input"
            ref="edit-name"
            :class="{ error: editError || editErrorMessage }"
            v-model.trim="fileName"
            size="small"
            :value="editValue"
            style="margin-left: 2px"
            @click.stop="() => {}"
            @focus.stop="() => {}"
            @change="changePath"
            @blur="handleEditFileBlur"
            @pressEnter="enterBlur"
          ></a-input>
          <a-input
            v-else-if="!isPending && !onEditing"
            ref="addPending"
            id="add-input"
            :class="{ error: editError || editErrorMessage }"
            v-model.trim="fileName"
            size="small"
            style="margin-left: 2px"
            @click.stop="() => {}"
            @focus.stop="() => {}"
            :value="addValue"
            @change="addChangePath"
            @blur="handleAddFileBlur"
            @pressEnter="enterBlur"
          ></a-input>
          <span
            class="text-overflow"
            v-show="
              fileNode &&
              (fileNode.isEntryFile ||
                fileNode.filePath === entryFile.filePath) &&
              fileNode.filePath !== undefined &&
              !onEditing
            "
          >
            ({{ $t('editor.entry_file') }})
          </span>
          <div
            class="deal-file"
            v-show="fileNode && !onEditing && fileNode.name && !fileNode.root"
          >
            <span
              class="mouse-over"
              :title="$t('rename')"
              @click.stop="handleRename"
            >
              <EditFilled class="icon" />
            </span>
            <span
              class="mouse-over"
              :title="$t('delete')"
              @click.stop="handleDelete"
            >
              <DeleteFilled class="icon" />
            </span>
          </div>
        </div>
        <span
          class="path"
          v-if="fileNode && fileNode.root"
          :title="fileNode.filePath"
        >
          {{ fileNode.filePath }}
        </span>
      </div>
      <div :style="{ 'padding-left': `${curCount * 16 + 26}px` }">
        <Alert
          class="error-message"
          :message="editErrorMessage"
          type="error"
          v-if="editError && editErrorMessage"
        />
      </div>
    </div>
    <div v-if="isShowChildren">
      <div v-for="id in fileNode.children.folder">
        <ComFileNode
          :fileNode="fileTree[id]"
          :id="id"
          type="folder"
          :count="childCount"
        />
      </div>
    </div>
    <div v-if="isShowChildren">
      <div v-for="id in fileNode.children.file">
        <ComFileNode
          :fileNode="fileTree[id]"
          :id="id"
          type="file"
          :count="childCount"
          @updateStrategyToApp="updateStrategyToApp"
        />
      </div>
    </div>
  </div>
</template>

<script lang="ts">
export default {
  name: 'ComFileNode',
  emits: ['updateStrategyToApp'],
};
</script>

<script setup lang="ts">
import { EditFilled, DeleteFilled } from '@ant-design/icons-vue';
import { useCodeStore } from '../store/codeStore';
import iconFolderJSON from '../config/iconFolderConfig.json';
import iconFileJSON from '../config/iconFileConfig.json';
import path from 'path';
import { storeToRefs } from 'pinia';
import {
  onMounted,
  ref,
  toRefs,
  computed,
  watch,
  nextTick,
  getCurrentInstance,
  ComponentInternalInstance,
} from 'vue';
import { Alert } from 'ant-design-vue';
import { openFolder } from '../../../assets/methods/codeUtils';
import {
  removeFileFolder,
  addFileSync,
} from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import fse from 'fs-extra';
import { ipcEmitDataByName } from '../../../ipcMsg/emitter';
import { confirmModal, messagePrompt } from '../../../assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

const { success, error, warning } = messagePrompt();
const { proxy } = getCurrentInstance() as ComponentInternalInstance;

const store = useCodeStore();

const props = defineProps<{
  fileNode: Code.FileData;
  type: string;
  id: number | string;
  count: number | string;
}>();

const { type, count } = props;
const { fileNode, id } = toRefs(props);
const curCount = ref<number>(+(count || 0));
const childCount = ref<number>(curCount.value + 1);

const iconPath = ref<string>('');
const fileName = ref<string>('');
const onEditing = ref<boolean>(false);
const editError = ref<boolean>(false);
const beforeDelate = ref<boolean>(true);
const editErrorMessage = ref<string>('');
const { fileTree, entryFile, currentFile } = storeToRefs(store);

const editValue = ref<string>('');
const addValue = ref<string>('');
const isShowChildren = computed(() => {
  return (
    fileNode.value &&
    fileNode.value.children &&
    fileNode.value.children.folder &&
    fileNode.value['open']
  );
});
const isPending = computed(() => {
  return fileTree.value['pending']['parentId'] === id.value;
});

watch(isShowChildren, () => {
  iconPath.value = getIcon(fileNode.value);
});

function enterBlur(e) {
  resetStatus();
  e.target.blur();
}

//点击文件或文件树
function handleClickFile(file) {
  resetStatus();
  //正在编辑的input
  if (Object.keys(file).length === 1) return;
  //更新active file id
  store.setCurrentFile(file);
  //如果为dir
  //打开文件夹, 如果children不为空，直接展示, 之后异步更新，将原来删除
  //如果children为空，读取文件夹下文件，赋值children
  if (type == 'folder' && !file.root) {
    openFolder(file, fileTree.value);
  }
}

//添加文件或文件夹时
const handleAddFileBlur = (e) => {
  resetStatus();
  e.stopPropagation();
  const filename = addValue.value;
  //test 重复 或 为空
  const parentId = fileNode.value?.parentId;
  if (parentId === null || parentId === undefined) return;
  const names = getSiblingsName(parentId);
  //如果为空则直接删除（重复会通过@input来判断）
  if (names.indexOf(filename) != -1 || !filename) {
    store.removeFileFolderPending({
      id: fileNode.value?.parentId,
      type: type,
    });
    addValue.value = '';
    return;
  }
  //添加文件
  try {
    const targetPath =
      parentId !== null && parentId !== undefined
        ? fileTree.value[parentId].filePath
        : '';
    const typeName = type == 'folder' ? t('folder') : t('file');

    if (type === 'folder' || type === 'file') {
      addFileSync(targetPath, filename, type);
    }
    store.removeFileFolderPending({
      id: fileNode.value?.parentId,
      type: type,
    });
    reloadFolder(parentId, filename);
    success(
      t('editor.creaate_success', {
        file: typeName,
      }),
    );
  } catch (err) {
    error((<Error>err).message || t('operation_failed'));
  }
  //重置
  resetStatus();
  addValue.value = '';
};

//添加/编辑输入检测
function handleAddEditFileInput(val): void {
  const siblings = getSiblingsName((fileNode.value as Code.FileData).parentId);
  const pattern = new RegExp('[\\ / : * ? " < > |]');
  if (siblings.indexOf(val) != -1) {
    editError.value = true;
    editErrorMessage.value = t('editor.name_repeat', {
      name: val,
    });
  } else if (!val) {
    editError.value = true;
    editErrorMessage.value = t('editor.empty_input');
  } else if (pattern.test(val)) {
    editError.value = true;
    editErrorMessage.value = t('editor.illegal_character');
  } else {
    editError.value = false;
    editErrorMessage.value = '';
  }
}

//重命名文件
function handleRename(): void {
  onEditing.value = true;
  nextTick(() => {
    document.getElementById('edit-input')?.focus();
    editValue.value = fileNode.value.name;
  });
}

//删除文件
function handleDelete() {
  if (
    !fileNode.value.isEntryFile ||
    fileNode.value.filePath !== entryFile.value.filePath
  ) {
    const parentId = fileNode.value?.parentId;
    const typeName = type == 'folder' ? t('folder') : t('file');

    confirmModal(t('prompt'), t('editor.delate_confirm')).then((flag) => {
      if (!flag) return;

      removeFileFolder(fileNode.value?.filePath || '')
        .then(() => {
          store.setCurrentFile(entryFile.value);
        })
        .then(() =>
          openFolder(fileTree.value[parentId || 0], fileTree.value, true, true),
        )
        .then(() => success(`${typeName}${t('operation_success')}`))
        .catch((err) => {
          if (err == 'cancel') return;
          error(err.message || t('operation_failed'));
        });
    });
  } else {
    warning(t('editor.cannot_delate_entry'));
    return;
  }
}

function changePath(e): void {
  const value = e.target.value;
  handleAddEditFileInput(value);
  editValue.value = value;
}
function addChangePath(e): void {
  const value = e.target.value;
  handleAddEditFileInput(value);
  addValue.value = value;
}

//重命名文件blur
const handleEditFileBlur = () => {
  onEditing.value = false;
  if (editValue.value == fileNode.value.name) {
    return;
  }
  if (!editValue.value || editError.value) {
    resetStatus();
    editValue.value = fileNode.value.name;
    return;
  }
  resetStatus();
  const oldPath = fileNode.value?.filePath || '';
  const newName = editValue.value;
  const newPath = path.join(path.dirname(oldPath), newName);
  const parentId = fileNode.value?.parentId;

  // 更改文件名
  fse
    .rename(oldPath, newPath)
    .then(() => {
      reloadFolder(parentId, newName);
    })
    .then(() => {
      if (fileNode.value === entryFile.value || fileNode.value.isEntryFile) {
        ipcEmitDataByName('updateStrategyPath', {
          strategyId: store.currentStrategy.strategy_id,
          strategyPath: newPath,
        }).then(() => {
          updateStrategyToApp(newPath);
        });
      }
    });
  editValue.value = '';
};

function updateStrategyToApp(newPath) {
  proxy?.$emit('updateStrategyToApp', newPath);
}

//重制状态
function resetStatus(): void {
  fileName.value = '';
  editErrorMessage.value = '';
  onEditing.value = false;
  editError.value = false;
}

// 重新加载folder
function reloadFolder(parentId, filename) {
  openFolder(fileTree.value[parentId], fileTree.value, true, true).then(
    (fileTree) => {
      const currentFile = getCurrentFileByName(parentId, fileTree, filename);
      if (currentFile.id) {
        store.setCurrentFile(currentFile);
      }
    },
  );
}

function getIcon(file: Code.FileData): string {
  let iconName: string = '';
  if (type == 'folder') {
    if (file['open']) {
      if (iconFolderJSON[file.name]) {
        iconName = iconFolderJSON[file.name] + '-open';
      } else {
        iconName = 'folder-open';
      }
    } else {
      iconName = iconFolderJSON[file.name];
      if (!iconName) iconName = 'folder';
    }
  } else {
    const ext: string = file.ext || '';
    const fileName: string = file.name || '';
    if (ext && iconFileJSON[ext]) {
      iconName = iconFileJSON[ext];
    } else {
      iconName = iconFileJSON[fileName];
    }
    if (!iconName) iconName = 'file';
  }
  if (!iconName) return '';
  return require(`@kungfu-trader/kungfu-app/src/renderer/assets/svg/file-icons/${iconName}.svg`);
}

//添加策略之后，刷新文件树，需要将current 更新为添加的file/folder
function getCurrentFileByName(parentId, fileTree, name) {
  const siblings = getSiblings(parentId, fileTree);
  return siblings[name] || {};
}

// 获取所有兄弟 name
function getSiblings(parentId: number | string, fileTree: Code.IFileTree) {
  const folders: Array<number | 'pending'> =
    fileTree[parentId].children['folder'] || [];
  const files: Array<number | 'pending'> =
    fileTree[parentId].children['file'] || [];
  return [...folders, ...files].reduce((pre, cur) => {
    if (fileTree[cur] && fileTree[cur].name) {
      pre[fileTree[cur].name] = fileTree[cur];
      return pre;
    }
    return pre;
  }, {} as Record<string, Code.FileData>);
}

//获取所有兄弟 name
function getSiblingsName(parentId) {
  if (fileTree.value[parentId] && fileTree.value[parentId].children) {
    const folders = fileTree.value[parentId].children['folder'] || [];
    const files = fileTree.value[parentId].children['file'] || [];
    return [...folders, ...files].reduce((pre, cur) => {
      if (fileTree.value[cur] && fileTree.value[cur].name) {
        pre.push(fileTree.value[cur].name);
        return pre;
      } else {
        return pre;
      }
    }, [] as Array<string>);
  }
  return [];
}

onMounted(() => {
  nextTick(() => {
    if (fileNode) {
      iconPath.value = getIcon(fileNode.value);
    }
    if (document.getElementById('add-input')) {
      document.getElementById('add-input')?.focus();
    }
  });
  //缓存filename
  fileName.value = fileNode ? fileNode.value.name : '';
});
</script>

<style lang="less">
.c-app-code-file-node {
  .ant-alert-error {
    border: 1px solid @red-base !important;
    background-color: #8d3333 !important;
  }
  .each-files {
    text-align: left;
    padding: 2px 0px;
    padding-left: 5px;
    color: @text-color;
    font-size: 14px;
    white-space: normal;
    cursor: pointer;
    &.root-file {
      background-color: #1d1f21;
      color: @gold-base;
    }
    .file-top {
      display: flex;
      flex-wrap: nowrap;
      align-items: center;
    }
    .text-overflow {
      flex: 1;
      white-space: nowrap;
      margin-right: 10px;
    }
    .path {
      margin: 0 4px;
      word-break: normal;
      width: auto;
      display: block;
      white-space: pre-wrap;
      word-wrap: break-word;
      overflow: hidden;
    }
    .file-icon {
      width: 20px;
      height: 20px;
      display: inline;
    }
    .file-name {
      margin: 0 4px;
      white-space: nowrap;
      text-overflow: ellipsis;
      overflow: hidden;
      word-break: break-all;
      &.normal-name {
        flex: 1;
      }
      &.root-name {
        font-size: 18px;
      }
    }
    &:hover {
      background-color: @popover-customize-border-color;
      color: @gold-base;
      .deal-file {
        display: block;
        white-space: nowrap;
      }
    }
    .editError {
      border: 1px solid red1-base;
    }
  }
  .active {
    background-color: @popover-customize-border-color;
    color: @gold-base;
  }
  .deal-file {
    margin-right: 5px;
    text-align: right;
    width: 60px;
    display: none;
    .mouse-over {
      margin: 0 2px;
      .icon {
        color: @text-color;
        &:hover {
          color: @icon-color-hover;
        }
      }
    }
  }
  .error-message {
    width: 100%;
    z-index: 999;
    padding: 5px 10px;
    box-sizing: border-box;
    line-height: 12px;
    font-size: 12px;
    border-top: none;
    text-align: left;
  }
}
</style>
