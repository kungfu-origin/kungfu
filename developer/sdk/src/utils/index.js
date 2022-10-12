const path = require('path');
const os = require('os');

const isProduction = () => process.env.NODE_ENV === 'production';

const isWin = os.platform() === 'win32';

const execSuffix = isWin ? '.exe' : '';

const kfcName = 'kfc' + execSuffix;

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

const dealPath = (pathname) => {
  return pathname.replace(/\\/g, '/').replace(/ /g, '\\ '); //保持斜杠而不是反斜杠，跟cmake模板内路径一致
};

const dealCmdPath = (pathname) => {
  if (os.platform() === 'win32') {
    return pathname
      .replace(/\\/g, '/')
      .split('/')
      .map((str) => {
        if (str.includes(' ')) {
          return `"${str}"`;
        }

        return str;
      })
      .join('/');
  }
  return dealPath(pathname);
};

const getKfcPath = () => {
  const pathMap = {
    [ModeMap.IN_CORE]: process.env.KFC_PATH,
    [ModeMap.IN_PROD_APP]: path.join(__dirname, '..', '..', '..', 'kfc'),
    [ModeMap.IN_SDK_SRC]: path.join(
      customResolve('@kungfu-trader/kungfu-core'),
      '..',
      '..',
      'dist',
      'kfc',
    ),
  };

  return pathMap[getCurrentMode()];
};

const getKfcCmdArgs = () => {
  const cmdMap = {
    [ModeMap.IN_CORE]: { cmd: 'yarn', args0: ['kfc'] },
    [ModeMap.IN_PROD_APP]: {
      cmd: dealCmdPath(path.resolve(getKfcPath(), kfcName)),
      args0: [],
    },
    [ModeMap.IN_SDK_SRC]: { cmd: 'yarn', args0: ['kfc'] },
  };

  return cmdMap[getCurrentMode()];
};

const getCmakeCmdArgs = () => {
  const cmdMap = {
    [ModeMap.IN_CORE]: { cmd: 'yarn', args: ['cmake-js', 'build'] },
    [ModeMap.IN_PROD_APP]: {
      cmd: 'cmake',
      args: ['-S', './', '-B', './build', '-DCMAKE_BUILD_TYPE=Release'],
    },
    [ModeMap.IN_SDK_SRC]: { cmd: 'yarn', args: ['cmake-js', 'build'] },
  };

  return cmdMap[getCurrentMode()];
};

const getCmakeNextCmdArgs = () => {
  const cmdMap = {
    [ModeMap.IN_CORE]: null,
    [ModeMap.IN_PROD_APP]: {
      cmd: 'cmake',
      args: ['--build', './build', '--config', 'Release'],
    },
    [ModeMap.IN_SDK_SRC]: null,
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

    const args = result.details.args;
    const firstOptionIndex = args.findIndex((item) => item.indexOf('-') !== -1);
    const parsedArgIndex = args
      .slice(0, firstOptionIndex === -1 ? args.length : firstOptionIndex)
      .findIndex((item) => tarCmds.indexOf(item) !== -1);

    if (parsedArgIndex === -1) {
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
  kfcName,
  dealPath,
  getKfcPath,
  getKfcCmdArgs,
  getCmakeCmdArgs,
  getCmakeNextCmdArgs,
  customResolve,
  parseByCli,
};
