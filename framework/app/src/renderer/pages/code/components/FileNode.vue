<template>
    <div>
        <div @click.stop="handleClickFile(fileNode)">
            <span>
                <i v-if="type == 'folder' && fileNode && !fileNode.root"></i>
                <img :src="iconPath" v-if="iconPath">
                <span v-if="id !== 'pedding' &&  fileNode && !onEditing">{{ fileNode.name }}</span>
                <span v-if="fileNode && entryFile.filePath === fileNode.filePath && fileNode.filePath !== undefined && !onEditing">{{ '入口文件' }}</span>
                <span v-if="fileNode && fileNode.root" :title="fileNode.filePath">{{fileNode.filePath}}</span>
                <span v-if="fileNode && !fileNode.root && !onEditing && id !== 'padding'">
                    <i class="nouse-over" title="重命名" @click="handleRename"></i>
                    <i class="nouse-over" title="删除" @click="handleDelete"></i>
                </span>
            </span>
        </div>
        <div v-if="fileNode && fileNode.children && fileNode.children.folder">
            <div v-for="id in fileNode.children.folder">
                <ComFileNode
                    :fileNode="fileTree[id]"
                    :id="id"
                    type="folder"
                    :key="id"
                />
            </div>
        </div>
        <div v-if="fileNode && fileNode.children && fileNode.children.file">
            <div v-for="id in fileNode.children.file">
                <ComFileNode
                    :fileNode="fileTree[id]"
                    :id="id"
                    type="file"
                    :key="id"
                />
            </div>
        </div>
    </div>
</template>

<script lang="ts">
export default {
    name: 'ComFileNode'
}
</script>


<script setup lang="ts">
import { useCodeStore } from '../store/codeStore'
import iconFolderJSON from '../config/iconFolderConfig.json';
import iconFileJSON from '../config/iconFileConfig.json';
import path from 'path';
import { storeToRefs } from 'pinia';
import { onMounted, PropType, ref, nextTick } from 'vue';
import { message } from 'ant-design-vue';
import { openFolder } from '../../../assets/methods/codeUtils';
import { removeFileFolder, editFileFolderName, addFileSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils'

const store = useCodeStore();
// const { currentFile, fileTree, kfConfig } = storeToRefs(store)
const props = defineProps({
    fileNode: Object as PropType<Code.FileData>,
    type: String,
    id: [Number, String]
})
const { fileNode, type } = props

const iconPath = ref<string>('');
const fileName = ref<string>('');
const onEditing = ref<boolean>(false)
const editError = ref<boolean>(false)
const editErrorMessage = ref<string>('');

const { fileTree, entryFile } = storeToRefs(store)

// const nameContent = ref<HTMLElement | null>(null)
const addPending = ref<HTMLElement | null>(null)
const editName = ref<HTMLElement | null>(null)

//点击文件或文件树
function handleClickFile(file) {
    //正在编辑的input
    if (Object.keys(file).length === 1) return;
    //更新active file id
    store.setCurrentFile(file)
    //如果为dir
    //打开文件夹, 如果children不为空，直接展示, 之后异步更新，将原来删除
    //如果children为空，读取文件夹下文件，赋值children
    if (type == 'folder' && !file.root) {
        openFolder(file, fileTree.value);
    }
}

//添加文件或文件夹时
function handleAddFileBlur(e, eType) {
    e.stopPropagation();
    if (eType === 'enter' && e.key !== 'Enter') return;
    const filename = e.target.value;
    //test 重复 或 为空
    const parentId = fileNode?.parentId || '';
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
        const targetPath = parentId ? fileTree.value[parentId].filePath : '';
        if (type === 'folder' || type ==='file') {
            addFileSync(targetPath, filename, type);
        }
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
        // 添加高亮
        if (editName.value) {
            editName.value.querySelectorAll('input')[0].focus();
        }
    });
}

//删除文件
function handleDelete() {
    const parentId = fileNode?.parentId;
    const typeName = type == 'folder' ? '文件夹' : '文件';
    this.$confirm(`确认删除该${typeName}吗？`, '提示', {
    confirmButtonText: '确 定',
    cancelButtonText: '取 消',
    })
    .then(() => removeFileFolder(fileNode?.filePath || ''))
    .then(() =>
        openFolder(
            fileTree.value[parentId || 0],
            fileTree.value,
            true,
            true,
        ),
    )
    .then(() =>
        store.setCurrentFile(fileTree.value[parentId || 0]),
    )
    .then(() => message.success(`${typeName}删除成功！`))
    .catch((err) => {
        if (err == 'cancel') return;
        message.error(err.message || '操作失败！');
    });
}

//重命名文件blur
function handleEditFileBlur(e) {
    if (editError) {
        resetStatus();
        return;
    }
    const oldPath = fileNode?.filePath || '';
    const newName = e.target.value;
    const newPath = path.join(path.dirname(oldPath), newName);
    const parentId = fileNode?.parentId;
    editFileFolderName(oldPath, newPath).then(() => {
        reloadFolder(parentId, newName);
    });
}

//重制状态
function resetStatus(): void {
    fileName.value = fileNode?.name || '';
    onEditing.value = false;
    editError.value = false;
    editErrorMessage.value = '';
}

// 重新加载folder
function reloadFolder(parentId, filename) {
    openFolder(
        fileTree.value[parentId],
        fileTree.value,
        true,
        true,
    ).then((fileTree) => {
        const currentFile = getCurrentFileByName(
            parentId,
            fileTree,
            filename,
        );
        if (currentFile.id) {
            store.setCurrentFile(currentFile)
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
    return `public/file-icons/${iconName}.svg`;
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
    const folders = fileTree.value[parentId].children['folder'];
    const files = fileTree.value[parentId].children['file'];

    [...folders, ...files].forEach((id) => {
    if (fileTree.value[id] && fileTree.value[id].name)
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
    //添加高亮
    if (addPending.value) {
      addPending.value.querySelectorAll('input')[0].focus();
    }
    //缓存filename
    fileName.value = fileNode ? fileNode.name : ''
})
</script>
<style lang="scss"></style>
