import './injectGlobal';
import { initKfConfig } from '@kungfu-trader/kungfu-js-api/config';
import globalStorage from '@kungfu-trader/kungfu-js-api/utils/globalStorage';

initKfConfig();

globalStorage.setItem('ifNotFirstRunning', true);
