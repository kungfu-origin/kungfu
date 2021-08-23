const fs = require('fs');
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const rootDir = path.dirname(__dirname);

const getCommitVersion = () => {
  const gitDir = path.resolve(rootDir, '..', '..', '.git');
  let gitCommitVersion = 'latest';
  try {
    let gitHEAD = fs.readFileSync(path.join(gitDir, 'HEAD'), 'utf-8').trim(); // ref: refs/heads/develop
    if ((gitHEAD || '').split(': ').length <= 1) {
      gitCommitVersion = gitHEAD;
    } else {
      let ref = gitHEAD.split(': ')[1]; // refs/heads/develop
      let develop = gitHEAD.split('/')[2]; // 环境：develop
      let gitVersion = fs.readFileSync(path.join(gitDir, ref), 'utf-8').trim(); // git版本号，例如：6ceb0ab5059d01fd444cf4e78467cc2dd1184a66
      gitCommitVersion = develop + ': ' + gitVersion;
    }
  } catch (err) {
    console.error(err);
  }
  return gitCommitVersion.toString();
};

const getPythonVersion = () => {
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

const listDirSync = (filePath) => {
  return fs.readdirSync(filePath);
};

const getViewsConfig = () => {
  const appDir = path.dirname(require.resolve('@kungfu-trader/kungfu-app/package.json'));
  const viewsPath = path.resolve(appDir, 'src', 'renderer', 'views');
  const files = listDirSync(viewsPath);
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

exports.getCommitVersion = getCommitVersion;
exports.getPythonVersion = getPythonVersion;
exports.getViewsConfig = getViewsConfig;
exports.IsProduction = process.env.NODE_ENV === 'production';
