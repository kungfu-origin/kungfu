import './setEnv';
import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/index/App.vue';
import router from '@kungfu-trader/kungfu-app/src/renderer/pages/index/router';
import store from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store';
import {
  Layout,
  Tabs,
  Button,
  Menu,
  Card,
  Input,
  Table,
  Switch,
  ConfigProvider,
  Modal,
  Radio,
  Tag,
  Form,
  InputNumber,
  Select,
  Drawer,
  Empty,
  DatePicker,
  Checkbox,
  Spin,
  Skeleton,
  Tree,
  Statistic,
  Row,
  Col,
  TimePicker,
} from 'ant-design-vue';

import { beforeStartAll } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { delayMilliSeconds } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  Pm2ProcessStatusDetailData,
  Pm2ProcessStatusData,
  Pm2ProcessStatusTypes,
  startArchiveMakeTask,
  startGetProcessStatus,
  startLedger,
  startMaster,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';

import { watcher } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import { tradingDataSubject } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import bus from '@kungfu-trader/kungfu-js-api/utils/globalBus';

import VueVirtualScroller from 'vue-virtual-scroller';
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css';
import { useComponenets } from './useComponents';
import { kf } from '@kungfu-trader/kungfu-js-api/kungfu';
import { GlobalCategoryRegister } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiExtraLocationUtils';

const app = createApp(App);

app
  .use(store)
  .use(router)
  .use(Layout)
  .use(Tabs)
  .use(Row)
  .use(Col)
  .use(Button)
  .use(Menu)
  .use(Card)
  .use(Input)
  .use(Table)
  .use(Switch)
  .use(ConfigProvider)
  .use(Modal)
  .use(Radio)
  .use(Tag)
  .use(InputNumber)
  .use(Select)
  .use(Drawer)
  .use(Form)
  .use(Empty)
  .use(DatePicker)
  .use(TimePicker)
  .use(Checkbox)
  .use(Spin)
  .use(Skeleton)
  .use(Tree)
  .use(Statistic)
  .use(VueVirtualScroller);

//this sort ensure $useGlobalStore can be get in mounted callback
app.config.globalProperties.$bus = bus;
app.config.globalProperties.$tradingDataSubject = tradingDataSubject;
app.config.globalProperties.$useGlobalStore = useGlobalStore;
app.config.globalProperties.$globalCategoryRegister =
  new GlobalCategoryRegister();

useComponenets(app);

app.mount('#app');

const globalStore = useGlobalStore();

if (process.env.RELOAD_AFTER_CRASHED === 'false') {
  beforeStartAll()
    .then(() => {
      return startArchiveMakeTask((archiveStatus: Pm2ProcessStatusTypes) => {
        bus.next({
          tag: 'processStatus',
          name: 'archive',
          status: archiveStatus,
        });
      });
    })
    .then(() => startMaster(false))
    .catch((err) => console.error(err.message))
    .finally(() => {
      startGetProcessStatus(
        (res: {
          processStatus: Pm2ProcessStatusData;
          processStatusWithDetail: Pm2ProcessStatusDetailData;
        }) => {
          const { processStatus, processStatusWithDetail } = res;
          globalStore.setProcessStatus(processStatus);
          globalStore.setProcessStatusWithDetail(processStatusWithDetail);
        },
      );

      delayMilliSeconds(1000)
        .then(() => startLedger(false))
        .catch((err) => console.error(err.message));
    });
} else {
  // 崩溃后重开, 跳过archive过程
  bus.next({
    tag: 'processStatus',
    name: 'archive',
    status: 'waiting restart',
  });
  startGetProcessStatus(
    (res: {
      processStatus: Pm2ProcessStatusData;
      processStatusWithDetail: Pm2ProcessStatusDetailData;
    }) => {
      const { processStatus, processStatusWithDetail } = res;
      globalStore.setProcessStatus(processStatus);
      globalStore.setProcessStatusWithDetail(processStatusWithDetail);
    },
  );
}

window.watcher = watcher;
window.kungfu = kf;
