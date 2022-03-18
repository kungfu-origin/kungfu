<template>
    <div class="code-content">
      <!-- <FileTree
        :strategy="strategy"
        @updateStrategy="handleUpdateStrategy"
      ></FileTree> -->
      <Editor ref="code-editor"></Editor>
    </div>
</template>
<script lang="ts">
    export default {
        name: 'kungfu-code-editor'
    }
</script>
<script setup lang="ts">
import Editor from './components/MonacoEditor.vue';
// import FileTree from './components/FileTree';
// import MainContent from '@/components/Layout/MainContentForCode';
import { setTimeout } from 'timers';
import { remote, ipcRenderer } from 'electron';
import { onMounted, reactive } from 'vue';
import { useRouter } from 'vue-router';

const currentWin = remote.getCurrentWindow();
const paWin = currentWin.getParentWindow();

ipcRenderer.setMaxListeners(10);

// const ipcEmitDataByName = (
//   name: string,
//   postData?: any,
//   interval?: number,
// ): Promise<> => {
//   interval = interval || 5000;

//   if (!paWin) {
//     return Promise.reject(new Error('paWin is null!'));
//   }

//   return new Promise((resolve, reject) => {
//     paWin.webContents.send(`ipc-emit-${name}`, {
//       childWinId: currentWin.id,
//       params: Object.freeze(postData || {}),
//     });

//     let timer = setTimeout(() => {
//       reject(new Error(`ipc-emit-${name} timeout`));
//       ipcRenderer.removeAllListeners(`ipc-res-${name}`);
//       clearTimeout(timer);
//     }, interval);

//     ipcRenderer.on(`ipc-res-${name}`, (event: any, data: any) => {
//       resolve({ event, data });
//       ipcRenderer.removeAllListeners(`ipc-res-${name}`);
//       clearTimeout(timer);
//     });
//   });
// };

// let strategy: KungfuApi.KfConfig = reactive({})

// function setCurrentStrategy  () {
//     const strategyId: string | string[] = useRouter().currentRoute.value.query.id
//     updateStrategy(strategyId);
// }

function bindCloseWindowEvent() {
    this.shouldClose = false;

    window.onbeforeunload = (e) => {
        e.preventDefault(e);
        if (this.shouldClose) return undefined;
        const $textareaList = document.querySelectorAll('textarea');
        $textareaList.forEach(($textarea) => {
            $textarea && $textarea.blur();
        });
        this.shouldClose = true;
        setTimeout(() => {
            window.close();
        }, 100);
        return false;
    };
}

// async function updateStrategy(strategyId: string | string[]) {
//     const { data } = await ipcEmitDataByName('strategyById', { strategyId });
//     this.strategy = data[0];
// }

onMounted(() => {
    // setCurrentStrategy();
    bindCloseWindowEvent();
}),


export default {
  name: 'kungfu-code-editor',

  data() {
    return {
      strategy: {},
    };
  },

  components: {
    FileTree,
    Editor,
    MainContent,
  },

  mounted() {
    this.setCurrentStrategy();
    this.bindCloseWindowEvent();
  },

  destroyed() {
    window.blur();
  },

  methods: {
    //子组件对strategy做了更改，需要更新策略
    handleUpdateStrategy() {
      if (!this.strategy.strategy_id) return;
      this.updateStrategy(this.strategy.strategy_id);
    },

    setCurrentStrategy() {
      const strategyId = this.$route.params.id;
      this.updateStrategy(strategyId);
    },

    

    //更新strategy
    async updateStrategy(strategyId) {
      const { data } = await ipcEmitDataByName('strategyById', { strategyId });
      this.strategy = data[0];
    },
  },
};
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.code-content {
  height: 100%;
  // background: $bg_card;
  display: flex;
  flex-direction: row;
}
</style>
