<template>
    
    <li 
    :key="fileNode.fileId"
    :class="{'folder': type == 'folder', 'file': type !== 'folder', 'active': fileNode.id === currentFile.id}"
    >   
        <div :class="{
            'file-name-content': true, 
            'on-editing': id === 'pending' || onEditing}" 
            @click.stop="handleClickFile(fileNode)"      
            ref="name-content">
            <span :class="{'is-root': fileNode.root}">
                <i :class="{
                    'el-icon-caret-right': !fileNode.open, 
                    'el-icon-caret-bottom': fileNode.open, 
                    'open': fileNode.open, }" 
                v-if="type == 'folder' && !fileNode.root"                    
                ></i>
                <img class="icon" v-if="iconPath" :src="iconPath"/>
                <!-- 文件名／编辑 -->
                <span 
                v-if="id != 'pending' && fileNode && !onEditing" 
                :class="[
                    'name', 
                    'text-overflow', 
                    ((entryFile.filePath === fileNode.filePath && fileNode.filePath !== undefined) ? 'import-file' : '')
                ]" 
                :title="fileNode.name"
                >{{fileNode.name}}</span>
                <el-input 
                v-else-if="id === 'pending'"
                ref="add-pending" 
                :class="{'error': editError}" 
                v-model.trim="fileName"
                @click.stop="() => {}"
                @focus.stop="() => {}"
                @keyup.enter.native="e => handleAddFileBlur(e, 'enter')"
                @blur="handleAddFileBlur" 
                @input="handleAddEditFileInput"/>                
                <el-input
                v-else-if="onEditing"
                ref="edit-name"
                :class="{'error': editError}" 
                v-model.trim="fileName"
                @click.stop="() => {}"
                @focus.stop="() => {}"
                @keyup.enter.native="handleEditFileBlur"
                @blur="handleEditFileBlur"
                @input="e => handleAddEditFileInput(e, 'edit')"
                ></el-input>
                <span class="name" v-if="entryFile.filePath === fileNode.filePath && fileNode.filePath !== undefined && !onEditing">{{'（入口文件）'}}</span>
                <!-- path or rename + delete-->
                <span :title="fileNode.filePath" class="path text-overflow" v-if="fileNode.root">{{fileNode.filePath}}</span>
                <span class="fr file-oper" v-if="!fileNode.root && !onEditing && id !== 'pending'">
                    <i class="el-icon-edit-outline mouse-over" title="重命名" @click.stop="handleRename(fileNode)"></i>
                    <i class="el-icon-close mouse-over" title="删除" @click.stop="handleDelete(fileNode)"></i>
                </span>
                <div class="error-message" v-if="editError">{{editErrorMessage}}</div>
            </span>
        </div>
        <ul v-if="fileNode.children && fileNode.open">
            <file-node v-for="id in fileNode.children.folder" :fileNode="fileTree[id]" :id="id" type="folder" :key="id"></file-node>
            <file-node v-for="id in fileNode.children.file" :fileNode="fileTree[id]" :id="id" type="file" :key="id"></file-node>
        </ul>

    </li>
</template>
<script>
import {mapState} from 'vuex';
import iconFolderJSON from '../config/iconFolderConfig.json'
import iconFileJSON from '../config/iconFileConfig.json'
import path from 'path';
import * as CODE_UTILS from "__gUtils/fileUtils.js";
import { error } from 'util';

export default {
    props: {
        fileNode: {
            default: {},
            type: Object
        },

        id: {
            default: '',
            type: [String, Number],
        },

        type: {
            default: '',
            type: String
        }
    },
    
    name: 'file-node',
    data(){
        return {
            fileName: '',
            iconPath: '',
            onEditing: false,//正在命名
            editError: false,
            editErrorMessage: '',
            timeoutId: '',
        }
    },

    computed: {
        ...mapState({
            currentFile: state => state.STRATEGY.currentFile,
            entryFile: state => state.STRATEGY.entryFile,
            fileTree: state => state.STRATEGY.fileTree,
        })
    },

    mounted(){
        const t = this;
        t.$nextTick().then(() => {t.iconPath = t.getIcon(t.fileNode)})    
        //添加高亮
        if(t.$refs['add-pending']){
            t.$refs['add-pending'].$el.querySelectorAll('input')[0].focus();
        }
        //缓存filename
        t.fileName = t.fileNode ? t.fileNode.name : '';
    },

    methods: {
         //点击文件或文件树
        handleClickFile(file){
            const t = this;
            //正在编辑的input
            if(Object.keys(file).length === 1) return
            //更新active file id
            t.$store.dispatch('setCurrentFile', file)
            //如果为dir
            //打开文件夹, 如果children不为空，直接展示, 之后异步更新，将原来删除
            //如果children为空，读取文件夹下文件，赋值children
            if(t.type == 'folder' && !file.root){
                CODE_UTILS.openFolder(file, t.fileTree)
            }

        },

        //添加文件或文件夹时
        handleAddFileBlur(e, type){
             e.stopPropagation();
            if(type === 'enter' && e.key !== 'Enter') return;
            const t = this;
            const filename = e.target.value;
            //test 重复 或 为空
            const parentId = t.fileNode.parentId;
            if(parentId === '') return;
            const names = t.getSiblingsName(parentId)
            //如果为空则直接删除（重复会通过@input来判断）
            if(names.indexOf(filename) != -1 || !filename){
                t.$store.dispatch('removeFileFolderPending', {id: t.fileNode.parentId, type: t.type})
                return
            }
            //添加文件
            try{
                const targetPath = t.fileTree[parentId].filePath;
                CODE_UTILS.addFile(targetPath, filename, t.type)
                t.$store.dispatch('removeFileFolderPending', {id: t.fileNode.parentId, type: t.type})
                t.reloadFolder(parentId, filename)
            }catch(err){
                t.$message.error(err.message || '操作失败！')
            }
            //重置
            t.resetStatus()
        },

        //添加/编辑输入检测
        handleAddEditFileInput(val, type){
            const t = this;
            const siblings = t.getSiblingsName(t.fileNode.parentId)
            const pattern = new RegExp("[\\ / : * ? \" < > |]");
            if(siblings.indexOf(val) != -1){
                if(type == 'edit' && val === t.fileNode.name) {
                    return;
                }
                t.editError = true;
                t.editErrorMessage = `此位置已存在文件或文件夹 ${val} ，请选择其他名称！`
            }else if(!val){
                t.editError = true;
                t.editErrorMessage = '必须提供文件或文件夹名称！'
            }else if(pattern.test(val)){
                t.editError = true;
                t.editErrorMessage = '名称不能包含\\/:*?"<>|'

            }else{
                t.editError = false;
                t.editErrorMessage = ''
            }
        },

        //重命名文件
        handleRename(){
            const t = this;
            t.onEditing = true;
            t.$nextTick().then(() => {
                //添加高亮
                if(t.$refs['edit-name']){
                    t.$refs['edit-name'].$el.querySelectorAll('input')[0].focus();
                }
            })
        },

        //重命名文件blur
        handleEditFileBlur(e){
            const t = this;
            if(t.editError){
                t.resetStatus()
                return;
            }
            const oldPath = t.fileNode.filePath;
            const newName = e.target.value;
            const newPath =  path.join(path.dirname(oldPath), newName)
            const parentId = t.fileNode.parentId;
            CODE_UTILS.editFileFolderName(oldPath, newPath).then(() => {
                t.reloadFolder(parentId, newName)
            })
        },


        //删除文件
        handleDelete(){
            const t = this;
            const parentId = t.fileNode.parentId;
            const filename = t.fileNode.name;
            const typeName = t.type == 'folder' ? '文件夹' : '文件'
            t.$confirm(`确认删除该${typeName}吗？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => CODE_UTILS.removeFileFolder(t.fileNode.filePath))
            .then(() => CODE_UTILS.openFolder(t.fileTree[parentId], t.fileTree, true, true))
            .then(() => t.$store.dispatch('setCurrentFile', t.fileTree[parentId]))
            .then(() => t.$message.success(`${typeName}删除成功！`)) 
            .catch((err) => {
                if(err == 'cancel') return
                t.$message.error(err.message || '操作失败！')
            })
        },

        //重制状态
        resetStatus(){
            const t = this;
            t.fileName = t.fileNode.name;
            t.onEditing = false;
            t.editError = false;
            t.editErrorMessage = '';
        },

        //重新加载folder
        reloadFolder(parentId, filename){
            const t = this;
            CODE_UTILS.openFolder(t.fileTree[parentId], t.fileTree, true, true).then((fileTree) => {
                const currentFile = t.getCurrentFileByName(parentId, fileTree, filename)
                if(currentFile.id){
                    t.$store.dispatch('setCurrentFile', currentFile)
                }
            })
        },

        //获取所有兄弟 name
        getSiblingsName(parentId){
            const t = this;
            let targetList = [];
            const folders =  t.fileTree[parentId].children['folder'];
            const files = t.fileTree[parentId].children['file'];


            [...folders, ...files].forEach(id => {
                if(t.fileTree[id] && t.fileTree[id].name) targetList.push(t.fileTree[id].name)
            })
            return targetList
        },

        //添加策略之后，刷新文件树，需要将current 更新为添加的file/folder
        getCurrentFileByName(parentId, fileTree, name){
            const t = this;
            const siblings = t.getSiblings(parentId, fileTree);
            return siblings[name] || {}
        },

        //获取所有兄弟 name
        getSiblings(parentId, fileTree){
            const t = this;
            let target = {};
            const folders =  fileTree[parentId].children['folder'];
            const files = fileTree[parentId].children['file'];

            [...folders, ...files].forEach(id => {
                if(fileTree[id] && fileTree[id].name) target[fileTree[id].name] = fileTree[id]
            })
            return target
        },

        getIcon(file){
            const t = this;
            let pathName, iconName;
            if(t.type == 'folder'){
                iconName = iconFolderJSON[file.name]
                if(!iconName) iconName = 'folder'
            }else{
                const ext = file.ext || '';
                const fileName = file.name || '';
                if(ext && iconFileJSON[ext]){
                    iconName = iconFileJSON[ext]
                }else{
                    iconName = iconFileJSON[fileName]
                }

                if(!iconName) iconName = 'file'
            }

            if(!iconName) return;
            return `resources/file-icons/${iconName}.svg`
        }
    }
}
</script>
<style lang="scss">
  
</style>

