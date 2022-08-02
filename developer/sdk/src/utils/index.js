const path = require('path');
const os = require('os');

const isProduction = () => process.env.NODE_ENV === 'production';

const isWin = os.platform() === 'win32';

const execSuffix = isWin ? '.exe' : '';

const kfcName = 'kfc' + execSuffix;
const cmakeName = 'cmake-js' + execSuffix;

const prodNodeMoudlesDir = path.join(__dirname, '..', '..', 'node_modules');

const customResolve = (path) => {
  if (isProduction()) {
    // eslint-disable-next-line no-undef
    return __non_webpack_require__.resolve(path);
  } else {
    return require.resolve(path);
  }
};

const ModeMap = {
  IN_CORE: 0,
  IN_PROD_APP: 1,
  IN_SDK_SRC: 2,
};

const getCurrentMode = () => {
  if (isProduction()) {
    if (process.env.KFC_PATH) {
      return ModeMap.IN_CORE;
    } else {
      return ModeMap.IN_PROD_APP;
    }
  } else {
    return ModeMap.IN_SDK_SRC;
  }
};

const getKfcPath = () => {
  const pathMap = {
    [ModeMap.IN_CORE]: process.env.KFC_PATH,
    [ModeMap.IN_PROD_APP]: path.join(__dirname, '..', '..', '..', 'kfc'),
    [ModeMap.IN_SDK_SRC]: path
      .join(
        customResolve('@kungfu-trader/kungfu-core'),
        '..',
        '..',
        'dist',
        'kfc',
      )
      .replace(/\\/g, '/'),
  };

  return pathMap[getCurrentMode()];
};

const getKfcCmdArgs = () => {
  const cmdMap = {
    [ModeMap.IN_CORE]: { cmd: 'yarn', args0: ['kfc'] },
    [ModeMap.IN_PROD_APP]: {
      cmd: path.join(getKfcPath(), kfcName),
      args0: [],
    },
    [ModeMap.IN_SDK_SRC]: { cmd: 'yarn', args0: ['kfc'] },
  };

  return cmdMap[getCurrentMode()];
};

const getCmakeCmdArgs = () => {
  const cmdMap = {
    [ModeMap.IN_CORE]: { cmd: 'yarn', args0: ['cmake-js'] },
    [ModeMap.IN_PROD_APP]: {
      cmd: path.join(prodNodeMoudlesDir, 'cmake-js', 'bin', cmakeName),
      args0: [],
    },
    [ModeMap.IN_SDK_SRC]: { cmd: 'yarn', args0: ['cmake-js'] },
  };

  return cmdMap[getCurrentMode()];
};

module.exports = {
  isProduction,
  getKfcPath,
  getKfcCmdArgs,
  getCmakeCmdArgs,
  customResolve,
};
