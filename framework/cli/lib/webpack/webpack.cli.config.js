'use strict';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');
const {
  getWebpackExternals,
  getAppDir,
  getCliDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const appDir = getAppDir();
const cliDir = getCliDir();
const { getKungfuBuildInfo, isProduction } = toolkit.utils;
const { gitCommitVersion, pyVersion, buildTimeStamp } = getKungfuBuildInfo();

const webpackConfig = (argv) =>
  merge(toolkit.webpack.makeConfig(argv), {
    externals: getWebpackExternals(),
    entry: {
      index: path.join(cliDir, 'src', 'index.ts'),
      dzxy: path.join(cliDir, 'src', 'dzxy.ts'),
    },
    target: 'node',
  });

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __git_commit_version: `"${gitCommitVersion.toString()}"`,
      __python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"cli"',
      'process.env.LANG_ENV': '"en-US"',
      __build_timestamp: `"${buildTimeStamp.toString()}"`,
      __resources: '',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __git_commit_version: `"${gitCommitVersion.toString()}"`,
      __python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"cli"',
      'process.env.LANG_ENV': '"en-US"',
      __build_timestamp: `"${buildTimeStamp.toString()}"`,
      __resources: `"${path.join(appDir, 'public').replace(/\\/g, '\\\\')}"`,
    }),
  ],
};

module.exports = (argv) => {
  return merge(
    webpackConfig(argv),
    isProduction(argv) ? prodConfig : devConfig,
  );
};
