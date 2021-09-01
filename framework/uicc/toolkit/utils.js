const fs = require('fs');
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

exports.getCommitVersion = () => {
  let gitCommitVersion = 'latest';
  try {
    let buildInfoRaw = fs.readFileSync(
      require.resolve('@kungfu-trader/kungfu-core/dist/kfc/kungfubuildinfo.json'),
      'utf-8',
    );
    let buildInfo = JSON.parse(buildInfoRaw);
    gitCommitVersion = buildInfo.git.revision;
  } catch (err) {
    console.error(err);
  }
  return gitCommitVersion.toString();
};

exports.getPythonVersion = () => {
  let pyVersion = '3';
  try {
    let buildInfoRaw = fs.readFileSync(
      require.resolve('@kungfu-trader/kungfu-core/dist/kfc/kungfubuildinfo.json'),
      'utf-8',
    );
    let buildInfo = JSON.parse(buildInfoRaw);
    pyVersion = buildInfo.pythonVersion || '3';
  } catch (err) {
    console.error(err);
  }
  return pyVersion;
};

exports.getViewsConfig = () => {
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
            nodeModules: process.env.NODE_ENV !== 'production' ? path.resolve(appDir, 'node_modules') : false,
          }),
        );
    });

  return {
    entry,
    plugins,
  };
};

exports.isProduction = () => process.env.NODE_ENV === 'production';
