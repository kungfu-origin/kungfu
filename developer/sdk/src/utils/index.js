const path = require('path');
const os = require('os');

const isProduction = () => process.env.NODE_ENV === 'production';

const isWin = os.platform() === 'win32';

const execSuffix = isWin ? '.exe' : '';

const kfcName = 'kfc' + execSuffix;

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
    console.log(
      'process.env.KFC_PATH',
      process.env.KFC_PATH,
      typeof process.env.KFC_PATH,
    );
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
    [ModeMap.IN_PROD_APP]: path
      .join(__dirname, '..', '..', '..', 'kfc')
      .replace(/\\/g, '/'),
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
      cmd: 'node',
      args0: [path.join(prodNodeMoudlesDir, 'cmake-js', 'bin', 'cmake-js')],
    },
    [ModeMap.IN_SDK_SRC]: { cmd: 'yarn', args0: ['cmake-js'] },
  };

  return cmdMap[getCurrentMode()];
};

const parseByCli = (cli, isRootCli = false) => {
  const tarCmds = cli.types.map((item) => item._aliases).flat(1);

  const exitHandler = (result) => {
    const checkError = () => {
      if (isRootCli) {
        if (result.output) {
          console.log(result.output);
          process.exit(result.code);
        }
        if (result.code !== 0) process.exit(result.code);
      }
    };

    const curArg = result.details.args.slice(-1)[0];

    if (tarCmds.indexOf(curArg) === -1) {
      cli.showHelpByDefault().parseAndExit().then(checkError);
    } else {
      checkError();
    }

    return result.argv;
  };

  const errorHandler = (error) => {
    console.error(error);
    process.exit(-1);
  };

  cli.parse().then(exitHandler).catch(errorHandler);
};

module.exports = {
  isProduction,
  getKfcPath,
  getKfcCmdArgs,
  getCmakeCmdArgs,
  customResolve,
  parseByCli,
};
