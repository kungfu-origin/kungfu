'use strict';

process.env.BABEL_ENV = 'main';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const rootDir = path.dirname(__dirname);
const baseConfig = toolkit.webpack.makeConfig(rootDir, 'app');
const { getKungfuBuildInfo, isProduction } = toolkit.utils;
const { gitCommitVersion, pyVersion, buildTimeStamp } = getKungfuBuildInfo();

const webpackConfig = merge(baseConfig, {
  entry: {
    main: path.join(rootDir, 'src', 'main', 'index.js'),
  },

  resolve: {
    alias: {
      '@': path.resolve(rootDir, 'src', 'renderer'),
    },
  },
  target: 'electron-main',
});

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      git_commit_version: `"${gitCommitVersion.toString()}"`,
      python_version: `"${pyVersion.toString()}"`,
      build_timestamp: `"${buildTimeStamp.toString()}"`,
      'process.env.APP_TYPE': '"main"',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      git_commit_version: `"${gitCommitVersion.toString()}"`,
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"main"',
      __resources: `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
    }),
  ],
};

module.exports = merge(webpackConfig, isProduction() ? prodConfig : devConfig);
