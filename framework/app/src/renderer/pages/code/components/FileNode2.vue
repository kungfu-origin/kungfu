<template>
  <li
    :key="fileNode.fileId"
    :class="{
      folder: type == 'folder',
      file: type !== 'folder',
      active: fileNode.id === currentFile.id,
    }"
  >
    <div
      :class="{
        'file-name-content': true,
        'on-editing': id === 'pending' || onEditing,
      }"
      @click.stop="handleClickFile(fileNode)"
      ref="name-content"
    >
      <span :class="{ 'is-root': fileNode.root }">
        <i
          :class="{
            'el-icon-caret-right': !fileNode.open,
            'el-icon-caret-bottom': fileNode.open,
            open: fileNode.open,
          }"
          v-if="type == 'folder' && !fileNode.root"
        ></i>
        <img class="icon" v-if="iconPath" :src="iconPath" />
        <!-- 文件名／编辑 -->
        <span
          v-if="id != 'pending' && fileNode && !onEditing"
          :class="[
            'name',
            'text-overflow',
            entryFile.filePath === fileNode.filePath &&
            fileNode.filePath !== undefined
              ? 'import-file'
              : '',
          ]"
          :title="fileNode.name"
        >
          {{ fileNode.name }}
        </span>
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
          class="name"
          v-if="
            entryFile.filePath === fileNode.filePath &&
            fileNode.filePath !== undefined &&
            !onEditing
          "
        >
          {{ '（入口文件）' }}
        </span>
        <!-- path or rename + delete-->
        <span
          :title="fileNode.filePath"
          class="path text-overflow"
          v-if="fileNode.root"
        >
          {{ fileNode.filePath }}
        </span>
        <span
          class="fr file-oper"
          v-if="!fileNode.root && !onEditing && id !== 'pending'"
        >
          <i
            class="el-icon-edit-outline mouse-over"
            title="重命名"
            @click.stop="handleRename(fileNode)"
          ></i>
          <i
            class="el-icon-close mouse-over"
            title="删除"
            @click.stop="handleDelete(fileNode)"
          ></i>
        </span>
        <div class="error-message" v-if="editError">{{ editErrorMessage }}</div>
      </span>
    </div>
    <ul v-if="fileNode.children && fileNode.open">
      <file-node
        v-for="id in fileNode.children.folder"
        :fileNode="fileTree[id]"
        :id="id"
        type="folder"
        :key="id"
      ></file-node>
      <file-node
        v-for="id in fileNode.children.file"
        :fileNode="fileTree[id]"
        :id="id"
        type="file"
        :key="id"
      ></file-node>
    </ul>
  </li>
</template>

<script setup lang="ts">
import { mapState } from 'vuex';
import iconFolderJSON from '../config/iconFolderConfig.json';
import iconFileJSON from '../config/iconFileConfig.json';
import path from 'path';
import * as CODE_UTILS from '__gUtils/fileUtils';

</script>