<template>
  <div
    v-if="beforeDelate"
    class="c-app-code-file-node"
    :class="{
      folder: type == 'folder',
      file: type !== 'folder',
      active: fileNode.id === currentFile.id,
    }"
  >
    <div @click.stop="handleClickFile(fileNode)">
      <div
        class="each-files"
        :style="{ 'padding-left': `${curCount * 16 + 5}px` }"
      >
        <i v-if="type == 'folder' && fileNode && !fileNode.root"></i>
        <img class="file-icon" :src="iconPath" v-if="iconPath" />
        <span
          class="file-name"
          :class="{ 'root-name': fileNode.root }"
          v-if="fileNode && !onEditing && fileNode.name"
        >
          {{ fileNode.name }}
        </span>
        <a-input
          v-else-if="onEditing"
          id="edit-input"
          ref="edit-name"
          :class="{ error: editError }"
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
          :class="{ error: editError }"
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
          v-if="
            fileNode &&
            entryFile.filePath === fileNode.filePath &&
            fileNode.filePath !== undefined &&
            !onEditing
          "
        >
          {{ '(入口文件)' }}
        </span>
        <span
          class="path text-overflow"
          v-if="fileNode && fileNode.root"
          :title="fileNode.filePath"
        >
          {{ fileNode.filePath }}
        </span>
        <span
          class="deal-file"
          v-if="fileNode && !fileNode.root && !onEditing && id !== 'padding'"
        >
          <span class="mouse-over" title="重命名" @click.stop="handleRename">
            <EditFilled class="icon" />
          </span>
          <span class="mouse-over" title="删除" @click.stop="handleDelete">
            <DeleteFilled class="icon" />
          </span>
        </span>
      </div>
      <Alert
        class="error-message"
        :message="editErrorMessage"
        type="error"
        v-if="editError"
        style="{'padding-left': `${curCount * 16 + 20}px`}"
      />
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
        />
      </div>
    </div>
  </div>
</template>

<script lang="ts">
export default {
  name: 'ComFileNode',
};
</script>

<script setup lang="ts">
import { EditFilled, DeleteFilled } from '@ant-design/icons-vue';
import { useCodeStore } from '../store/codeStore';
import iconFolderJSON from '../config/iconFolderConfig.json';
import iconFileJSON from '../config/iconFileConfig.json';
import path from 'path';
import { storeToRefs } from 'pinia';
import { onMounted, ref, toRefs, computed, watch, nextTick } from 'vue';
import { message, Modal, Alert } from 'ant-design-vue';
import { openFolder } from '../../../assets/methods/codeUtils';
import {
  removeFileFolder,
  addFileSync,
} from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import fse from 'fs-extra';
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
  e.target.blur()
  resetStatus()
}

//点击文件或文件树
function handleClickFile(file) {
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
    return;
  }
  //添加文件
  try {
    const targetPath =
      parentId !== null && parentId !== undefined
        ? fileTree.value[parentId].filePath
        : '';
    const typeName = type == 'folder' ? '文件夹' : '文件';

    if (type === 'folder' || type === 'file') {
      addFileSync(targetPath, filename, type);
    }
    store.removeFileFolderPending({
      id: fileNode.value?.parentId,
      type: type,
    });
    reloadFolder(parentId, filename);
    message.success(`${typeName}创建成功！`);
  } catch (err) {
    message.error(err.message || '操作失败！');
  }
  //重置
  resetStatus();
};

//添加/编辑输入检测
function handleAddEditFileInput(val): void {
  const siblings = getSiblingsName((fileNode.value as Code.FileData).parentId);
  const pattern = new RegExp('[\\ / : * ? " < > |]');
  if (siblings.indexOf(val) != -1) {
    editError.value = true;
    editErrorMessage.value = `此位置已存在文件或文件夹 ${val} ，请选择其他名称！`;
  } else if (!val) {
    editError.value = true;
    editErrorMessage.value = '必须提供文件或文件夹名称！';
  } else if (pattern.test(val)) {
    editError.value = true;
    editErrorMessage.value = '名称不能包含\\/:*?"<>|';
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
  });
}

//删除文件
function handleDelete() {
    if (entryFile.value.id != fileNode.value.id) {
        const parentId = fileNode.value?.parentId;
        const typeName = type == 'folder' ? '文件夹' : '文件';
        Modal.confirm({
            title: '提示',
            content: `确认删除该${typeName}吗？`,
            okText: '确 定',
            cancelText: '取 消',
            onOk() {
                removeFileFolder(fileNode.value?.filePath || '').then(() => {
                    store.setCurrentFile(entryFile.value)
                }).then(() =>
                    openFolder(
                        fileTree.value[parentId || 0],
                        fileTree.value,
                        true,
                        true,
                    ),
                )
                .then(() => message.success(`${typeName}删除成功！`))
                .catch((err) => {
                    if (err == 'cancel') return;
                    message.error(err.message || '操作失败！');
                });
            },
            onCancel() {
                return
            }
        })
    } else {
        message.warning('不可删除入口文件')
        return
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
  if (editError) {
    resetStatus();
  }

  const oldPath = fileNode.value?.filePath || '';
  const newName = editValue.value;
  const newPath = path.join(path.dirname(oldPath), newName);
  const parentId = fileNode.value?.parentId;

  // 更改文件名
  fse.rename(oldPath, newPath).then(() => {
    reloadFolder(parentId, newName);
  });
};

//重制状态
function resetStatus(): void {
  fileName.value = fileNode.value?.name || '';
  onEditing.value = false;
  editError.value = false;
  editErrorMessage.value = '';
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
        iconName = iconFolderJSON[file.name] + 'open';
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
function getSiblings(parentId, fileTree) {
  let target = {};
  const folders = fileTree[parentId].children['folder'];
  const files = fileTree[parentId].children['file'];

  [...folders, ...files].forEach((id) => {
    if (fileTree[id] && fileTree[id].name)
      target[fileTree[id].name] = fileTree[id];
  });
  return target;
}

//获取所有兄弟 name
function getSiblingsName(parentId) {
  let targetList: Array<string> = [];
  if (fileTree.value[parentId] && fileTree.value[parentId].children) {
    const folders = fileTree.value[parentId].children['folder'] || [];
    const files = fileTree.value[parentId].children['file'] || [];

    [...folders, ...files].forEach((id) => {
      if (fileTree.value[id] && fileTree.value[id].name)
        targetList.push(fileTree.value[id].name);
    });
    return targetList;
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
  .each-files {
    display: flex;
    margin-right: 20px;
    padding: 2px 0px;
    padding-left: 5px;
    color: @text-color;
    font-size: 14px;
    align-items: center;
    white-space: normal;
    cursor: pointer;
    .text-overflow {
      &.path {
        display: block;
      }
    }
    .file-icon {
      width: 20px;
      height: 20px;
    }
    .file-name {
      margin: 0 4px;
      &.root-name {
        font-size: 18px;
      }
    }
    &:hover {
      background-color: @popover-customize-border-color;
      color: @gold-base;
      .deal-file {
        display: block;
      }
    }
  }
  .active {
    .each-files {
      background-color: @popover-customize-border-color;
      color: @gold-base;
    }
  }
  .deal-file {
    margin-right: 5px;
    flex: 1;
    text-align: right;
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
    width: calc(100% - 34px);
    z-index: 999;
    padding: 5px 10px;
    box-sizing: border-box;
    line-height: 12px;
    font-size: 9px;
    margin-top: 5px;
    margin-left: 15px;
  }
}
</style>
