
const fs = require("fs");
const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const getCommitVersion = () => {
  var gitCommitVersion = 'latest'
  try {
    var gitHEAD = fs.readFileSync(path.join(__dirname, '..', '..', '.git', 'HEAD'), 'utf-8').trim() // ref: refs/heads/develop
    if((gitHEAD || '').split(': ').length <= 1) {
      gitCommitVersion = gitHEAD
    } else {
      var ref = gitHEAD.split(': ')[1] // refs/heads/develop
      var develop = gitHEAD.split('/')[2] // 环境：develop
      var gitVersion = fs.readFileSync(path.join(__dirname, '..', '..', '.git', ref), 'utf-8').trim() // git版本号，例如：6ceb0ab5059d01fd444cf4e78467cc2dd1184a66
      gitCommitVersion = develop + ': ' + gitVersion
    }
  } catch (err) {
    console.error(err)
  }
  return gitCommitVersion.toString();
}


const getPythonVersion = () => {
  var pyVersion = "3";
  try {
    var buildInfoRaw = fs.readFileSync(path.join(__dirname, '..', '..', 'core', 'build', 'kfc', 'kungfubuildinfo.json'), 'utf-8')
    var buildInfo = JSON.parse(buildInfoRaw);
    pyVersion = buildInfo.pythonVersion || "3"
  } catch (err) {
    console.error(err)
  }
  return pyVersion
}

const listDirSync = (filePath) => {
  return fs.readdirSync(filePath)
}

const getViewsConfig = () => {
  const viewsPath = path.resolve(__dirname, "..", "src", "renderer", "views");
  console.log(viewsPath)
  const files = listDirSync(viewsPath);
  let entry = {}, plugins = [];
  files
  .filter(file => {
    const filePath = path.join(viewsPath, file);
    const stats = fs.statSync(filePath);
    return stats.isDirectory();
  })
  .forEach(file => {
    entry[file] = path.join(__dirname, `../src/renderer/views/${file}/main.js`),
    plugins.push( new HtmlWebpackPlugin({
      filename: `${file}.html`,
      template: path.resolve(__dirname, '../src/index.ejs'),
      minify: {
        collapseWhitespace: true,
        removeAttributeQuotes: true,
        removeComments: true
      },
      chunks: [file],
      nodeModules: process.env.NODE_ENV !== 'production'
        ? path.resolve(__dirname, '../node_modules')
        : false
    }))
  });

  return {
    entry,
    plugins
  }
}

exports.getCommitVersion = getCommitVersion;
exports.getPythonVersion = getPythonVersion;
exports.getViewsConfig = getViewsConfig;