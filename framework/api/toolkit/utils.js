const fs = require('fs-extra');
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const findRoot = require('find-root');

exports.getKungfuBuildInfo = () => {
  try {
    const buildInfoRaw = fs.readFileSync(
      require.resolve(
        '@kungfu-trader/kungfu-core/dist/kfc/kungfubuildinfo.json',
      ),
      'utf-8',
    );
    const buildInfo = JSON.parse(buildInfoRaw);
    const pyVersion = buildInfo.pythonVersion;
    const gitCommitVersion = buildInfo.git.revision;
    const buildTimeStamp = buildInfo.build.timestamp;

    return {
      pyVersion,
      gitCommitVersion,
      buildTimeStamp,
    };
  } catch (err) {
    console.error(err);
    throw err;
  }
};

exports.getPagesConfig = (argv) => {
  const appDir = this.getAppDir();
  const pagesDir = path.resolve(appDir, 'src', 'renderer', 'pages');
  const files = fs.readdirSync(pagesDir);
  let entry = {},
    plugins = [];
  files
    .filter((file) => {
      const filePath = path.join(pagesDir, file);
      const stats = fs.statSync(filePath);
      return stats.isDirectory();
    })
    .forEach((dir) => {
      entry[dir] = path.resolve(pagesDir, dir, 'main.ts');
      plugins.push(
        new HtmlWebpackPlugin({
          title: '功夫交易系统',
          filename: `${dir}.html`,
          template: path.resolve(appDir, 'public', 'index.ejs'),
          minify: {
            collapseWhitespace: true,
            removeAttributeQuotes: true,
            removeComments: true,
          },
          chunks: [dir],
          nodeModules: !this.isProduction(argv)
            ? path.resolve(appDir, 'node_modules')
            : false,
        }),
      );
    });

  return {
    entry,
    plugins,
  };
};

exports.getComponentsConfig = () => {
  const appDir = this.getAppDir();
  const componentsDir = path.resolve(appDir, 'src', 'components', 'modules');
  const files = fs.readdirSync(componentsDir);
  let entry = {};

  files
    .filter((file) => {
      const filePath = path.join(componentsDir, file);
      const stats = fs.statSync(filePath);
      return stats.isDirectory();
    })
    .forEach((dir) => {
      entry[dir] = path.resolve(componentsDir, dir, 'index.ts');
    });

  return entry;
};

exports.getWebpackExternals = () => {
  const appPackageJSONPath = require.resolve(
    '@kungfu-trader/kungfu-app/package.json',
  );
  const apiPackageJSONPath = require.resolve(
    '@kungfu-trader/kungfu-js-api/package.json',
  );
  const corePackageJSONPath = require.resolve(
    '@kungfu-trader/kungfu-core/package.json',
  );
  const sdkPackageJSONPath = require.resolve(
    '@kungfu-trader/kungfu-sdk/package.json',
  );
  const currentPackageJSONPath = path.join(process.cwd(), 'package.json');
  const appPackageJSON = fs.readJSONSync(appPackageJSONPath);
  const apiPackageJSON = fs.readJSONSync(apiPackageJSONPath);
  const corePackageJSON = fs.readJSONSync(corePackageJSONPath);
  const sdkPackageJSON = fs.readJSONSync(sdkPackageJSONPath);
  const currentPackageJSON = fs.pathExistsSync(currentPackageJSONPath)
    ? fs.readJSONSync(currentPackageJSONPath)
    : {};
  return Object.keys({
    ...(appPackageJSON.dependencies || {}),
    ...(apiPackageJSON.dependencies || {}),
    ...(corePackageJSON.dependencies || {}),
    ...(sdkPackageJSON.dependencies || {}),
    ...(currentPackageJSON.dependencies || {}),
  }).filter(
    (item) => !item.includes('kungfu-js-api') || !item.includes('kungfu-core'),
  );
};

exports.getAppDefaultDistDir = () => {
  return path.resolve(this.getAppDir(), 'dist');
};

exports.getCliDefaultDistDir = () => {
  return path.resolve(this.getCliDir(), 'dist');
};

exports.getSdkDefaultDistDir = () => {
  return path.resolve(this.getSdkDir(), 'dist');
};

exports.getAppDir = () => {
  return path.dirname(
    require.resolve('@kungfu-trader/kungfu-app/package.json'),
  );
};

exports.getApiDir = () => {
  return path.dirname(
    require.resolve('@kungfu-trader/kungfu-js-api/package.json'),
  );
};

exports.getCliDir = () => {
  try {
    return path.dirname(
      require.resolve('@kungfu-trader/kungfu-cli/package.json'),
    );
  } catch (err) {
    return '.';
  }
};

exports.getCoreDir = () => {
  return path.dirname(
    require.resolve('@kungfu-trader/kungfu-core/package.json'),
  );
};

exports.getKfcDir = () => {
  return path.join(
    path.dirname(require.resolve('@kungfu-trader/kungfu-core/package.json')),
    'dist',
    'kfc',
  );
};

exports.getSdkDir = () => {
  return path.dirname(
    require.resolve('@kungfu-trader/kungfu-sdk/package.json'),
  );
};

exports.isProduction = (argv) => argv.mode === 'production';

exports.getExtensionDirs = (production = false) => {
  const packageJSON = fs.readJSONSync(
    path.resolve(process.cwd(), 'package.json'),
  );

  return [
    ...Object.keys(packageJSON.dependencies || {}),
    ...Object.keys(packageJSON.optionalDependencies || {}),
    ...(production ? [] : Object.keys(packageJSON.devDependencies || {})),
  ]
    .map((name) => {
      let fullPath = '';
      try {
        const jsonPath = require.resolve(name + '/package.json');
        const json = fs.readJSONSync(jsonPath);
        if (json.kungfuConfig) {
          fullPath = path.dirname(jsonPath);
        }
      } catch (err) {}
      return fullPath;
    })
    .filter((fullpath) => !!fullpath);
};

exports.getKungfuConfigKey = () => {
  return require(path.join(process.cwd(), 'package.json'))['kungfuConfig'][
    'key'
  ];
};

exports.buildDevArgv = (distDir, distName) => {
  const cliDir = exports.getCliDir();
  const appDir = exports.getAppDir();
  const kfcDir = exports.getKfcDir();
  const extdirs = exports.getExtensionDirs();

  process.env.KFC_DIR = kfcDir;
  process.env.APP_PUBLIC_DIR = path.join(appDir, 'public');
  process.env.CLI_DIR = path.join(cliDir, 'dist', 'cli');
  process.env.KFC_DEV = 'true';
  process.env.EXTENSION_DIRS = [distDir, ...extdirs].join(path.delimiter);

  return {
    mode: 'development',
    distDir: distDir,
    distName: distName,
  };
};

exports.findPackageRoot = () => {
  const cwd = process.cwd();
  if (cwd.includes('node_modules')) {
    return findRoot(path.resolve(cwd.split('node_modules')[0]));
  }
  return findRoot(path.resolve(cwd));
};
