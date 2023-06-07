import { CLI_DIR, KFC_DIR, KF_HOME, KF_RUNTIME_DIR } from './config/pathConfig';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { dealSpaceInPath } from './utils/busiUtils';
import { KUNGFU_SAFE_CPUS_NUM, getCpusNum } from './utils/osUtils';
import {
  getKfGlobalSettingsValue,
  setKfGlobalSettingsValue,
} from './config/globalSettings';
import { booleanProcessEnv } from './utils/commonUtils';
import globalStorage from './utils/globalStorage';

// 此文件为所有需要预置在进程时携带的环境变量
// 注意：由于前端 app 的渲染进程是由 main 进程启动，c++ 中通过 std::getenv 的方式只能获取进程启动时就带有的 env
// 所以需要在渲染进程启动前就挂载以下的环境变量，也就是在 main 进程中挂载

// 从项目配置的预置env
const packageJson = readRootPackageJsonSync();
const externalEnv = packageJson.kungfuCraft?.env;
if (externalEnv && typeof externalEnv === 'object') {
  Object.keys(externalEnv).forEach((key) => {
    const curEnvValue = externalEnv[key];
    if (typeof curEnvValue === 'string') process.env[key] = curEnvValue;
  });
}

process.env.KFC_DIR = dealSpaceInPath(KFC_DIR);
process.env.CLI_DIR = dealSpaceInPath(CLI_DIR);
process.env.KF_HOME = dealSpaceInPath(KF_HOME);
process.env.KF_RUNTIME_DIR = dealSpaceInPath(KF_RUNTIME_DIR);
process.env.PYTHONUTF8 = '1';
process.env.PYTHONIOENCODING = 'utf8';

process.env.CPUS_NUM = await getCpusNum();
process.env.IF_CPUS_NUM_SAFE = process.env.CPUS_NUM > KUNGFU_SAFE_CPUS_NUM;

if (!globalStorage.getItem('ifNotFirstRunning')) {
  if (!booleanProcessEnv(process.env.IF_CPUS_NUM_SAFE)) {
    const globalSettings = getKfGlobalSettingsValue();
    if (!globalSettings.performance) globalSettings.performance = {};
    globalSettings.performance.bypassAccounting = true;
    await setKfGlobalSettingsValue(globalSettings);
  }

  globalStorage.setItem('ifNotFirstRunning', true);
}
