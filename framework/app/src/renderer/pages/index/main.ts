import './setEnv';
import './injectWindow';
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
  Divider,
  Dropdown,
} from 'ant-design-vue';

import {
  postStartAll,
  preStartAll,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { delayMilliSeconds } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  Pm2ProcessStatusDetailData,
  Pm2ProcessStatusData,
  Pm2ProcessStatusTypes,
  startArchiveMakeTask,
  startGetProcessStatus,
  startLedger,
  startCacheD,
  startMaster,
  pm2LaunchBus,
  Pm2Packet,
  Pm2Bus,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';

import {
  tradingDataSubject,
  triggerStartStep,
} from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';

import VueVirtualScroller from 'vue-virtual-scroller';
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css';
import { useComponenets } from './useComponents';
import { GlobalCategoryRegister } from '@kungfu-trader/kungfu-js-api/utils/extraLocationUtils';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';

import VueI18n from '@kungfu-trader/kungfu-js-api/language';

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
  .use(Divider)
  .use(Dropdown)
  .use(VueVirtualScroller);

app.config.globalProperties.$globalBus = globalBus;
app.config.globalProperties.$tradingDataSubject = tradingDataSubject;
app.config.globalProperties.$globalCategoryRegister =
  new GlobalCategoryRegister();

app.use(VueI18n);
app.mount('#app');
useComponenets(app, router);

const globalStore = useGlobalStore();

pm2LaunchBus((err: Error, pm2_bus: Pm2Bus) => {
  if (err) {
    console.error('pm2 launchBus Error', err);
    return;
  }

  pm2_bus &&
    pm2_bus.on('process:msg', (packet: Pm2Packet) => {
      const { type, body } = packet.data;
      console.log(type, body);
      switch (type) {
        case 'GlobalBus':
          globalBus.next(body);
          break;
        default:
          break;
      }
    });
});

if (process.env.RELOAD_AFTER_CRASHED === 'false') {
  preStartAll()
    .then(() => {
      return startArchiveMakeTask((archiveStatus: Pm2ProcessStatusTypes) => {
        globalBus.next({
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
        .then(() => startCacheD(false))
        .then(() => delayMilliSeconds(1000))
        .then(() => startLedger(false))
        .then(() => postStartAll())
        .then(() => delayMilliSeconds(1000))
        .then(() => {
          globalBus.next({
            tag: 'processStatus',
            name: 'extraResourcesLoading',
            status: 'online',
          });
        })
        .catch((err) => console.error(err.message));
    });
} else {
  // 崩溃后重开, 跳过archive过程
  globalBus.next({
    tag: 'processStatus',
    name: 'archive',
    status: 'waiting restart',
  });

  globalBus.next({
    tag: 'processStatus',
    name: 'extraResourcesLoading',
    status: 'online',
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

triggerStartStep();
