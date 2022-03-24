<template>
  <li>
      <div>
          <span>
              <i></i>
              <!-- <img src="iconPath" alt="iconPath"> -->
              <span></span>
              <el-imput></el-imput>
              <span></span>

              <span></span>
              <span></span>
              <div class="error-message"></div>
          </span>
      </div>
  </li>
</template>

<script setup lang="ts">
import { useCodeStore } from '../store/codeStore'
import iconFolderJSON from '../config/iconFolderConfig.json';
import iconFileJSON from '../config/iconFileConfig.json';
// import path from 'path';
import { storeToRefs } from 'pinia';
import { onMounted, PropType, ref } from 'vue';
import { nextTick } from 'process';
import { message } from 'ant-design-vue';


const store = useCodeStore();
// const { currentFile, fileTree, kfConfig } = storeToRefs(store)
const props = defineProps({
    fileNode: Object as PropType<Code.FileData>,
    type: String
})
const { fileNode, type } = props

const iconPath = ref<string>('');
const fileName = ref<string>('');
const onEditing = ref<boolean>(false)
const editError = ref<boolean>(false)
const editErrorMessage = ref<string>('');

const { fileTree } = storeToRefs(store)


//点击文件或文件树
// function handleClickFile(file) {
//     //正在编辑的input
//     if (Object.keys(file).length === 1) return;
//     //更新active file id
//     this.$store.dispatch('setCurrentFile', file);
//     //如果为dir
//     //打开文件夹, 如果children不为空，直接展示, 之后异步更新，将原来删除
//     //如果children为空，读取文件夹下文件，赋值children
//     if (this.type == 'folder' && !file.root) {
//     CODE_UTILS.openFolder(this.$store, file, this.fileTree);
//     }
// }

//添加文件或文件夹时
function handleAddFileBlur(e, eType) {
    e.stopPropagation();
    if (eType === 'enter' && e.key !== 'Enter') return;
    const filename = e.target.value;
    //test 重复 或 为空
    const parentId = fileNode?.parentId;
    if (parentId === '') return;
    const names = getSiblingsName(parentId);
    //如果为空则直接删除（重复会通过@input来判断）
    if (names.indexOf(filename) != -1 || !filename) {
        store.removeFileFolderPending({
            id: fileNode?.parentId,
            type: type,
        })
        return;
    }
    //添加文件
    try {
        const targetPath = parentId ? fileTree[parentId].filePath : '';
        CODE_UTILS.addFileSync(targetPath, filename, type);
        store.removeFileFolderPending({
            id: fileNode?.parentId,
            type: type,
        })
        reloadFolder(parentId, filename);
    } catch (err) {
        message.error(err.message || '操作失败！');
    }
    //重置
    resetStatus();
}

//添加/编辑输入检测
function handleAddEditFileInput(val, type): void {
    const siblings = getSiblingsName((fileNode as Code.FileData).parentId);
    const pattern = new RegExp('[\\ / : * ? " < > |]');
    if (siblings.indexOf(val) != -1) {
        if (type == 'edit' && val === (fileNode as Code.FileData).name) {
            return;
        }
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
        //添加高亮
        // if (this.$refs['edit-name']) {
        //     this.$refs['edit-name'].$el.querySelectorAll('input')[0].focus();
        // }
    });
}

//重制状态
function resetStatus(): void {
    fileName.value = (fileNode as Code.FileData).name;
    onEditing.value = false;
    editError.value = false;
    editErrorMessage.value = '';
}

// 重新加载folder
function reloadFolder(parentId, filename) {
    CODE_UTILS.openFolder(9
    this.$store,
    this.fileTree[parentId],
    this.fileTree,
    true,
    true,
    ).then((fileTree) => {
    const currentFile = this.getCurrentFileByName(
        parentId,
        fileTree,
        filename,
    );
    if (currentFile.id) {
        this.$store.dispatch('setCurrentFile', currentFile);
    }
    });
}

function getIcon(file: Code.FileData): string {
    let iconName: string = '';
        if (type == 'folder') {
            iconName = iconFolderJSON[file.name];
            if (!iconName) iconName = 'folder';
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
    return `resources/file-icons/${iconName}.svg`;
}

//添加策略之后，刷新文件树，需要将current 更新为添加的file/folder
function getCurrentFileByName(parentId, fileTree, name) {
    const siblings = getSiblings(parentId, fileTree);
    return siblings[name] || {};
}

//获取所有兄弟 name
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
    let targetList: Array<string>[] = [];
    const folders = fileTree[parentId].children['folder'];
    const files = fileTree[parentId].children['file'];

    [...folders, ...files].forEach((id) => {
    if (fileTree[id] && fileTree[id].name)
        targetList.push(fileTree[id].name);
    });
    return targetList;
}

onMounted(() => {
    nextTick(() => {
        if (fileNode) {
            iconPath.value = getIcon(fileNode)
        }
    })
    
    fileName.value = fileNode ? fileNode.name : ''
})
</script>
<style lang="scss"></style>
