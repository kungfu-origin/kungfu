<template>
    <div
        v-if="beforeDelate"
        class="c-app-code-file-node"
        :class="{
            folder: type == 'folder',
            file: type !== 'folder',
            active: fileNode?.id === currentFile?.id,
        }"
    >
        <div @click.stop="handleClickFile(fileNode)">
            <div class="each-files" :style="{'padding-left': `${curCount * 16 + 5}px`}">
                <i v-if="type == 'folder' && fileNode && !fileNode.root"></i>
                <img class="file-icon" :src="iconPath" v-if="iconPath">
                <span class="file-name" v-if="id !== 'pedding' &&  fileNode && !onEditing && fileNode.name">{{ fileNode.name }}</span>

                <a-input
                    v-else-if="id === 'pending'"
                    ref="addPending"
                    :class="{ error: editError }"
                    v-model.trim="fileName"
                    size="small"
                    @click.stop="() => {}"
                    @focus.stop="() => {}"
                    :value="addValue"
                    @change="addChangePath"
                    @pressEnter="handleAddFileBlur"
                ></a-input>
                <a-input
                    v-else-if="onEditing"
                    ref="edit-name"
                    :class="{ error: editError }"
                    v-model.trim="fileName"
                    :defaultValue="fileNode?.name"
                    size="small"
                    :value="editValue"
                    @click.stop="() => {}"
                    @focus.stop="() => {}"
                    @change="changePath"
                    @pressEnter="handleEditFileBlur"
                ></a-input>

                <span class="path text-overflow" v-if="fileNode && entryFile.filePath === fileNode.filePath && fileNode.filePath !== undefined && !onEditing">{{ '(入口文件)' }}</span>
                <span class="path text-overflow" v-if="fileNode && fileNode.root" :title="fileNode.filePath">{{fileNode.filePath}}</span>
                <span class="deal-file"  v-if="fileNode && !fileNode.root && !onEditing && id !== 'padding'">
                    <span class="mouse-over" title="重命名" @click.stop="handleRename"><img src="../../../../../public/file-icons/edit.svg" alt=""></span>
                    <span class="mouse-over" title="删除" @click.stop="handleDelete"><img src="../../../../../public/file-icons/delate.svg" alt=""></span>
                </span>
            </div>
        </div>
        <div v-if="fileNode && fileNode.children && fileNode.children.folder && showChild">
            <div v-for="id in fileNode.children.folder">
                <ComFileNode
                    :fileNode="fileTree[id]"
                    :id="id"
                    type="folder"
                    :key="id"
                    :count="childCount"
                />
            </div>
        </div>
        <div v-if="fileNode && fileNode.children && fileNode.children.file && showChild">
            <div v-for="id in fileNode.children.file">
                <ComFileNode
                    :fileNode="fileTree[id]"
                    :id="id"
                    type="file"
                    :key="id"
                    :count="childCount"
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
import { message, Modal } from 'ant-design-vue';
import { openFolder } from '../../../assets/methods/codeUtils';
import { removeFileFolder, addFileSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils'
import fse from 'fs-extra'

const store = useCodeStore();
const props = defineProps({
    fileNode: Object as PropType<Code.FileData>,
    type: String,
    id: [Number, String],
    count: [Number, String]
})

const { fileNode, type, id, count } = props
const curCount = ref<number>(+(count || 0))
const childCount = ref<number>(curCount.value + 1)

const iconPath = ref<string>('');
const fileName = ref<string>('');
const onEditing = ref<boolean>(false)
const editError = ref<boolean>(false)
const showChild = ref<boolean>(true)
const beforeDelate = ref<boolean>(true)

const editErrorMessage = ref<string>('');

const editValue = ref<string>(fileNode?.name || '')
const addValue = ref<string>(fileNode?.name || '')

const { fileTree, entryFile, currentFile } = storeToRefs(store)

//点击文件或文件树
function handleClickFile(file) {
    // if (type === 'folder') {
    //     showChild.value = !showChild.value
    // }
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
function handleAddFileBlur(e) {
    e.stopPropagation();
    const filename = addValue.value
    //test 重复 或 为空
    const parentId = fileNode?.parentId;
    if (parentId === null || parentId === undefined) return;
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
        const targetPath = (parentId !== null && parentId !== undefined) ? fileTree.value[parentId].filePath : '';
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
// function handleAddEditFileInput(val, type): void {
    
//     const siblings = getSiblingsName((fileNode as Code.FileData).parentId);
//     const pattern = new RegExp('[\\ / : * ? " < > |]');
//     if (siblings.indexOf(val) != -1) {
//         if (type == 'edit' && val === (fileNode as Code.FileData).name) {
//             return;
//         }
//         editError.value = true;
//         editErrorMessage.value = `此位置已存在文件或文件夹 ${val} ，请选择其他名称！`;
//     } else if (!val) {
//         editError.value = true;
//         editErrorMessage.value = '必须提供文件或文件夹名称！';
//     } else if (pattern.test(val)) {
//         editError.value = true;
//         editErrorMessage.value = '名称不能包含\\/:*?"<>|';
//     } else {
//         editError.value = false;
//         editErrorMessage.value = '';
//     }
// }



//重命名文件
function handleRename(): void {
    onEditing.value = true;
}

//删除文件
function handleDelete() {
    const parentId = fileNode?.parentId;
    const typeName = type == 'folder' ? '文件夹' : '文件';
    Modal.confirm({
        title: '提示',
        content: `确认删除该${typeName}吗？`,
        okText: '确 定',
        cancelText: '取 消',
        onOk() {
            removeFileFolder(fileNode?.filePath || '').then(() =>
                openFolder(
                    fileTree.value[parentId || 0],
                    fileTree.value,
                    true,
                    true,
                ),
            )
            .then(() =>{
                store.setCurrentFile(entryFile.value)
            })
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
}

function changePath(e): void {
    const value = e.target.value
    editValue.value = value
}
function addChangePath(e): void {
    const value = e.target.value
    addValue.value = value
}

//重命名文件blur
function handleEditFileBlur(e) {
    // if (fileNode) {
    //     fileNode.name = editValue.value
    // }
    // editValue.value = fileNode?.name || ''
    onEditing.value = false
    // if (editError) {
    //     resetStatus();
    //     return;
    // }
    
    const oldPath = fileNode?.filePath || '';
    const newName = editValue.value
    const newPath = path.join(path.dirname(oldPath), newName);
    const parentId = fileNode?.parentId;
    
    // 更改文件名
    fse.rename(oldPath, newPath).then(() => {
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
    return require(`../../../../../public/file-icons/${iconName}.svg`);
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
    return []
}

onMounted(() => {
    nextTick(() => {
        if (fileNode) {
            iconPath.value = getIcon(fileNode)
            
        }
    })
    //添加高亮
    // if (addPending.value) {
    //   addPending.value.querySelectorAll('input')[0].focus();
    // }
    //缓存filename
    fileName.value = fileNode ? fileNode.name : ''
})
</script>

<style lang="less">
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/code.less';
.c-app-code-file-node {
    .each-files {
        display: flex;
        overflow: hidden;
        text-overflow: ellipsis;
        white-space: nowrap;
        margin-right: 20px;
        padding: 2px 0px;
        padding-left: 5px;
        color: @font_6;
        cursor: pointer;
        .text-overflow {
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }
        .file-icon {
            width: 20px;
            height: 20px;
        }
        .file-name {
            margin: 0 2px;
        }
        &:hover {
            background-color: @bg_light;
            color: @white;
            .deal-file {
                display: block;
            }
        }
    }
    .active {
        .each-files {
            background-color: @bg_light;
            color: @white;
        }
    }
    .deal-file {
        margin-right: 5px;
        flex: 1;
        text-align: right;
        display: none;
        .mouse-over {
            margin: 0 2px;
        }
    }
}
</style>
