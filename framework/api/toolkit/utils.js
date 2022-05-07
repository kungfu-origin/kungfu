const fs = require('fs-extra');
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

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
  const currentPackageJSONPath = path.join(process.cwd(), 'package.json');
  const appPackageJSON = fs.readJSONSync(appPackageJSONPath);
  const apiPackageJSON = fs.readJSONSync(apiPackageJSONPath);
  const currentPackageJSON = fs.pathExistsSync(currentPackageJSONPath)
    ? fs.readJSONSync(currentPackageJSONPath)
    : {};
  return Object.keys({
    ...appPackageJSON.dependencies,
    ...apiPackageJSON.dependencies,
    ...(currentPackageJSON.dependencies || {}),
  }).filter((item) => !item.includes('kungfu-js-api'));
};

exports.getAppDefaultDistDir = () => {
  return path.resolve(this.getAppDir(), 'dist');
};

exports.getCliDefaultDistDir = () => {
  return path.resolve(this.getCliDir(), 'dist');
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

exports.isProduction = (argv) => argv.mode === 'production';

exports.getExtensionDirs = (production = false) => {
  const packageJSON = fs.readJSONSync(
    path.resolve(process.cwd(), 'package.json'),
  );

  const extdirs = [
    ...Object.keys(packageJSON.dependencies || {}),
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

  return extdirs;
};

exports.getKungfuConfigKey = () => {
  return require(path.join(process.cwd(), 'package.json'))['kungfuConfig'][
    'key'
  ];
};
