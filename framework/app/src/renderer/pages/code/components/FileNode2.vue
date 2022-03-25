<template>
<div>
    <div
        @click.stop="handleClickFile(fileNode)"
        ref="name-content"
      >
          <span>
                <i v-if="type == 'folder' && fileNode && !fileNode.root"></i>
                <img src="iconPath" alt="iconPath">
                <!-- 文件名/编辑 -->
                <span
                    v-if="id !='pending' && fileNode && !onEditing"
                    :title="fileNode.name"
                >{{ fileNode.name }}</span>

                <el-input
                    v-else-if="id === 'pending'"
                    ref="add-pending"
                    :class="{ error: editError }"
                    v-model.trim="fileName"
                    @click.stop="() => {}"
                    @focus.stop="() => {}"
                    @keyup.enter.native="(e) => handleAddFileBlur(e, 'enter')"
                    @blur="handleAddFileBlur"
                    @input="handleAddEditFileInput"
                />
                <el-input
                    v-else-if="onEditing"
                    ref="edit-name"
                    :class="{ error: editError }"
                    v-model.trim="fileName"
                    @click.stop="() => {}"
                    @focus.stop="() => {}"
                    @keyup.enter.native="handleEditFileBlur"
                    @blur="handleEditFileBlur"
                    @input="(e) => handleAddEditFileInput(e, 'edit')"
                ></el-input>
                <span
                    v-if="
                        entryFile.filePath === fileNode?.filePath &&
                        fileNode?.filePath !== undefined &&
                        !onEditing"
                >{{'(入口文件)'}}</span>

                <span
                    :title="fileNode?.filePath"
                    class="path text-overflow"
                    v-if="fileNode?.root"
                >{{ fileNode.filePath }}}</span>

                <span
                    class="fr file-oper"
                    v-if="!fileNode?.root && !onEditing && id !== 'pending'"
                >
                    <i
                        class="mouse-over"
                        title="重命名"
                        @click.stop="handleRename()"
                    ></i>
                    <i
                        class="mouse-over"
                        title="删除"
                        @click.stop="handleDelete()"
                    ></i>
                </span>
                <div class="error-message"></div>
          </span>
    </div>
    <div>{{fileNode?.filePath}}</div>
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
import { mapState } from 'vuex';
import iconFolderJSON from '../config/iconFolderConfig.json';
import iconFileJSON from '../config/iconFileConfig.json';
import path from 'path';
import * as CODE_UTILS from '__gUtils/fileUtils';

export default {
  props: {
    fileNode: {
      default: {},
      type: Object,
    },

    id: {
      default: '',
      type: [String, Number],
    },

    type: {
      default: '',
      type: String,
    },
  },

  name: 'file-node',
  data() {
    return {
      fileName: '',
      iconPath: '',
      onEditing: false, //正在命名
      editError: false,
      editErrorMessage: '',
      timeoutId: '',
    };
  },

  computed: {
    ...mapState({
      currentFile: (state) => state.currentFile,
      entryFile: (state) => state.entryFile,
      fileTree: (state) => state.fileTree,
    }),
  },

  mounted() {
    this.$nextTick().then(() => {
      this.iconPath = this.getIcon(this.fileNode);
    });
    //添加高亮
    if (this.$refs['add-pending']) {
      this.$refs['add-pending'].$el.querySelectorAll('input')[0].focus();
    }
    //缓存filename
    this.fileName = this.fileNode ? this.fileNode.name : '';
  },

  methods: {
    //点击文件或文件树
    handleClickFile(file) {
      //正在编辑的input
      if (Object.keys(file).length === 1) return;
      //更新active file id
      this.$store.dispatch('setCurrentFile', file);
      //如果为dir
      //打开文件夹, 如果children不为空，直接展示, 之后异步更新，将原来删除
      //如果children为空，读取文件夹下文件，赋值children
      if (this.type == 'folder' && !file.root) {
        CODE_UTILS.openFolder(this.$store, file, this.fileTree);
      }
    },

    //添加文件或文件夹时
    handleAddFileBlur(e, type) {
      e.stopPropagation();
      if (type === 'enter' && e.key !== 'Enter') return;
      const filename = e.target.value;
      //test 重复 或 为空
      const parentId = this.fileNode.parentId;
      if (parentId === '') return;
      const names = this.getSiblingsName(parentId);
      //如果为空则直接删除（重复会通过@input来判断）
      if (names.indexOf(filename) != -1 || !filename) {
        this.$store.dispatch('removeFileFolderPending', {
          id: this.fileNode.parentId,
          type: this.type,
        });
        return;
      }
      //添加文件
      try {
        const targetPath = this.fileTree[parentId].filePath;
        CODE_UTILS.addFileSync(targetPath, filename, this.type);
        this.$store.dispatch('removeFileFolderPending', {
          id: this.fileNode.parentId,
          type: this.type,
        });
        this.reloadFolder(parentId, filename);
      } catch (err) {
        this.$message.error(err.message || '操作失败！');
      }
      //重置
      this.resetStatus();
    },

    //添加/编辑输入检测
    handleAddEditFileInput(val, type) {
      const siblings = this.getSiblingsName(this.fileNode.parentId);
      const pattern = new RegExp('[\\ / : * ? " < > |]');
      if (siblings.indexOf(val) != -1) {
        if (type == 'edit' && val === this.fileNode.name) {
          return;
        }
        this.editError = true;
        this.editErrorMessage = `此位置已存在文件或文件夹 ${val} ，请选择其他名称！`;
      } else if (!val) {
        this.editError = true;
        this.editErrorMessage = '必须提供文件或文件夹名称！';
      } else if (pattern.test(val)) {
        this.editError = true;
        this.editErrorMessage = '名称不能包含\\/:*?"<>|';
      } else {
        this.editError = false;
        this.editErrorMessage = '';
      }
    },

    //重命名文件
    handleRename() {
      this.onEditing = true;
      this.$nextTick().then(() => {
        //添加高亮
        if (this.$refs['edit-name']) {
          this.$refs['edit-name'].$el.querySelectorAll('input')[0].focus();
        }
      });
    },

    //重命名文件blur
    handleEditFileBlur(e) {
      if (this.editError) {
        this.resetStatus();
        return;
      }
      const oldPath = this.fileNode.filePath;
      const newName = e.target.value;
      const newPath = path.join(path.dirname(oldPath), newName);
      const parentId = this.fileNode.parentId;
      CODE_UTILS.editFileFolderName(oldPath, newPath).then(() => {
        this.reloadFolder(parentId, newName);
      });
    },

    //删除文件
    handleDelete() {
      const parentId = this.fileNode.parentId;
      const typeName = this.type == 'folder' ? '文件夹' : '文件';
      this.$confirm(`确认删除该${typeName}吗？`, '提示', {
        confirmButtonText: '确 定',
        cancelButtonText: '取 消',
      })
        .then(() => CODE_UTILS.removeFileFolder(this.fileNode.filePath))
        .then(() =>
          CODE_UTILS.openFolder(
            this.$store,
            this.fileTree[parentId],
            this.fileTree,
            true,
            true,
          ),
        )
        .then(() =>
          this.$store.dispatch('setCurrentFile', this.fileTree[parentId]),
        )
        .then(() => this.$message.success(`${typeName}删除成功！`))
        .catch((err) => {
          if (err == 'cancel') return;
          this.$message.error(err.message || '操作失败！');
        });
    },

    //重制状态
    resetStatus() {
      this.fileName = this.fileNode.name;
      this.onEditing = false;
      this.editError = false;
      this.editErrorMessage = '';
    },

    //重新加载folder
    reloadFolder(parentId, filename) {
      CODE_UTILS.openFolder(
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
    },

    //获取所有兄弟 name
    getSiblingsName(parentId) {
      let targetList = [];
      const folders = this.fileTree[parentId].children['folder'];
      const files = this.fileTree[parentId].children['file'];

      [...folders, ...files].forEach((id) => {
        if (this.fileTree[id] && this.fileTree[id].name)
          targetList.push(this.fileTree[id].name);
      });
      return targetList;
    },

    //添加策略之后，刷新文件树，需要将current 更新为添加的file/folder
    getCurrentFileByName(parentId, fileTree, name) {
      const siblings = this.getSiblings(parentId, fileTree);
      return siblings[name] || {};
    },

    //获取所有兄弟 name
    getSiblings(parentId, fileTree) {
      let target = {};
      const folders = fileTree[parentId].children['folder'];
      const files = fileTree[parentId].children['file'];

      [...folders, ...files].forEach((id) => {
        if (fileTree[id] && fileTree[id].name)
          target[fileTree[id].name] = fileTree[id];
      });
      return target;
    },

    getIcon(file) {
      let iconName;
      if (this.type == 'folder') {
        iconName = iconFolderJSON[file.name];
        if (!iconName) iconName = 'folder';
      } else {
        const ext = file.ext || '';
        const fileName = file.name || '';
        if (ext && iconFileJSON[ext]) {
          iconName = iconFileJSON[ext];
        } else {
          iconName = iconFileJSON[fileName];
        }

        if (!iconName) iconName = 'file';
      }

      if (!iconName) return;
      return `resources/file-icons/${iconName}.svg`;
    },
  },
};
</script>
<style lang="scss"></style>
