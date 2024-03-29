import './setEnv';
import './injectGlobal';
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
  Progress,
  Popover,
} from 'ant-design-vue';

import {
  postStartAll,
  preStartAll,
  mergeExtLanguages,
  checkCpusNumAndConfirmModal,
  loadCustomFont,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import {
  delayMilliSeconds,
  buildIfWatcherLiveObservable,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { booleanProcessEnv } from '@kungfu-trader/kungfu-js-api/utils/commonUtils';
import {
  Pm2ProcessStatusDetailData,
  Pm2ProcessStatusData,
  Pm2ProcessStatusTypes,
  startArchiveMakeTask,
  startGetProcessStatus,
  startLedger,
  startCacheD,
  startMaster,
  isAllMainProcessRunning,
  KillAll,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';

import {
  tradingDataSubject,
  triggerStartStep,
} from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';

import VueVirtualScroller from 'vue-virtual-scroller';
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css';
import { useComponents } from './useComponents';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';

import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import zhCN from 'ant-design-vue/es/locale/zh_CN';
import enUS from 'ant-design-vue/es/locale/en_US';
import { first } from 'rxjs';

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
  .use(Progress)
  .use(Popover)
  .use(VueVirtualScroller);

app.config.globalProperties.$antLocalesMap = {
  'zh-CN': zhCN,
  'en-US': enUS,
};
app.config.globalProperties.$globalBus = globalBus;
app.config.globalProperties.$tradingDataSubject = tradingDataSubject;

app.use(VueI18n);

loadCustomFont().then(() =>
  mergeExtLanguages().then(() =>
    useComponents(app, router).then(() => {
      app.mount('#app');
    }),
  ),
);

const globalStore = useGlobalStore();
const __BYPASS_ARCHIVE__ = false;
let appMounted = false;

globalBus.subscribe((data) => {
  if (data.tag === 'appMounted') {
    appMounted = true;
  }
});

const initStartAll = () => {
  const start = () => {
    preStartAll()
      .then(() => checkCpusNumAndConfirmModal())
      .then((res) => {
        return delayMilliSeconds(2000).then(() => {
          globalBus.next({
            tag: 'preStartCheck',
            name: 'cpusNum',
            status: res,
          });
        });
      })
      .then(async () => {
        if (__BYPASS_ARCHIVE__) {
          globalBus.next({
            tag: 'processStatus',
            name: 'archive',
            status: 'online',
          });
          await delayMilliSeconds(2000);
          globalBus.next({
            tag: 'processStatus',
            name: 'archive',
            status: 'stopped',
          });
          return;
        } else {
          return startArchiveMakeTask(
            (archiveStatus: Pm2ProcessStatusTypes) => {
              globalBus.next({
                tag: 'processStatus',
                name: 'archive',
                status: archiveStatus,
              });
            },
          );
        }
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
      });

    const watcherIsLiveObervable = buildIfWatcherLiveObservable(window.watcher);
    watcherIsLiveObervable.pipe(first()).subscribe(() => {
      console.log('watcher is live');
      delayMilliSeconds(2000)
        .then(() => startCacheD(false))
        .then(() => delayMilliSeconds(2000))
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
  };

  if (appMounted) {
    start();
  } else {
    globalBus.subscribe((data) => {
      if (data.tag === 'appMounted') {
        start();
      }
    });
  }
};

if (!booleanProcessEnv(process.env.RELOAD_AFTER_CRASHED)) {
  initStartAll();
} else {
  isAllMainProcessRunning().then((res) => {
    if (res) {
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
    } else {
      KillAll().finally(() => {
        initStartAll();
      });
    }
  });
}

triggerStartStep(1000);
