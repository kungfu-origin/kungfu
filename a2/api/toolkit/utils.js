const fs = require('fs-extra');
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

exports.getKungfuBuildInfo = () => {
  try {
    const buildInfoRaw = fs.readFileSync(
      require.resolve('@kungfu-trader/kungfu-core/dist/kfc/kungfubuildinfo.json'),
      'utf-8',
    );
    const buildInfo = JSON.parse(buildInfoRaw);
    const pyVersion = buildInfo.pythonVersion;
    const gitCommitVersion = buildInfo.git.revision;
    const buildTimeStamp = buildInfo.build.timestamp;
    
    return {
      pyVersion,
      gitCommitVersion,
      buildTimeStamp
    }
  } catch (err) {
    console.error(err);
    throw err;
  }
}



exports.getPagesConfig = (argv) => {
  const appConfigPath = require.resolve('@kungfu-trader/kungfu-app2/package.json');
  const appDir = path.dirname(appConfigPath);
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
          title: "功夫交易系统",
          filename: `${dir}.html`,
          template: path.resolve(appDir, 'public', 'index.ejs'),
          minify: {
            collapseWhitespace: true,
            removeAttributeQuotes: true,
            removeComments: true,
          },
          chunks: [dir],
          nodeModules: !this.isProduction(argv) ? path.resolve(appDir, 'node_modules') : false,
        }),
      );
    });

  return {
    entry,
    plugins,
  };
};

exports.getComponentsConfig = (argv) => {
  const appConfigPath = require.resolve('@kungfu-trader/kungfu-app2/package.json');
  const appDir = path.dirname(appConfigPath);
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
    })

  return entry;
}

exports.isProduction = (argv) => argv.mode === 'production';
