const os = require('os');
const path = require('path');

declare const global: NodeJS.Global;

const getHomePath = () => {
  switch (os.platform()) {
    case 'darwin':
      return path.join(
        os.homedir(),
        'Library',
        'Application Support',
        'kungfu',
      );
    case 'win32':
      return path.join(os.homedir(), 'AppData', 'Roaming', 'kungfu');
    case 'linux':
      return path.join(os.homedir(), '.config', 'kungfu');
  }
};

if (process.env.NODE_ENV === 'development') {
  global.__resources = `${__resources}`;
} else {
  global.__resources = path
    .resolve(__dirname, '../public')
    .replace(/\\/g, '\\\\');
  console.log('global.__resources', global.__resources);
}

if (process.env.APP_TYPE === 'main' || process.env.APP_TYPE === 'renderer') {
  //global.__kfResourcesPath 是一个容易出错的问题, 需要每个调用pathconfig的进程都注册了这个值, 不然报错
  global.__kfResourcesPath = process.resourcesPath;
}

if (process.env.APP_TYPE === 'cli') {
  global.__kfResourcesPath = path
    .resolve(__dirname, '..', '..')
    .replace(/\\/g, '\\\\');
  console.log('global.__kfResourcesPath', global.__kfResourcesPath);
}

export const KF_HOME_BASE_DIR_RESOLVE: string = getHomePath();
