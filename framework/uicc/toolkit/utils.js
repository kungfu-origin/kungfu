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



exports.getViewsConfig = (mode) => {
  const appConfigPath = require.resolve('@kungfu-trader/kungfu-app/package.json');
  const appDir = path.dirname(appConfigPath);
  const viewsPath = path.resolve(appDir, 'src', 'renderer', 'views');
  const files = fs.readdirSync(viewsPath);
  let entry = {},
    plugins = [];
  files
    .filter((file) => {
      const filePath = path.join(viewsPath, file);
      const stats = fs.statSync(filePath);
      return stats.isDirectory();
    })
    .forEach((dir) => {
      (entry[dir] = path.resolve(appDir, 'src', 'renderer', 'views', dir, 'main.js')),
        plugins.push(
          new HtmlWebpackPlugin({
            filename: `${dir}.html`,
            template: path.resolve(appDir, 'src', 'index.ejs'),
            minify: {
              collapseWhitespace: true,
              removeAttributeQuotes: true,
              removeComments: true,
            },
            chunks: [dir],
            nodeModules: !this.isProduction(mode) ? path.resolve(appDir, 'node_modules') : false,
          }),
        );
    });

  return {
    entry,
    plugins,
  };
};

exports.isProduction = (mode) => mode === 'production';
