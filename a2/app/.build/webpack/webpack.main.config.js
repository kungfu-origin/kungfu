'use strict';

process.env.BABEL_ENV = 'main';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const rootDir = path.dirname(path.dirname(__dirname));
const { getKungfuBuildInfo, isProduction } = toolkit.utils;
const { gitCommitVersion, pyVersion, buildTimeStamp } = getKungfuBuildInfo();

const ESLintPlugin = require('eslint-webpack-plugin');

const webpackConfig = (argv) => merge(
  toolkit.webpack.makeConfig(argv),
  {
    externals: ['fkill'],

    entry: {
      main: path.join(rootDir, 'src', 'main', 'index.ts'),
    },

    resolve: {
      alias: {
        '@root': rootDir,
        '@main': path.resolve(rootDir, 'src', 'main')
      }
    },
    target: 'electron-main'
  }
);

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __git_commit_version: `"${gitCommitVersion.toString()}"`,
      __python_version: `"${pyVersion.toString()}"`,
      __build_timestamp: `"${buildTimeStamp.toString()}"`,
      'process.env.APP_TYPE': '"main"',
    }),

    new ESLintPlugin({
        fix: true, /* 自动帮助修复 */
        extensions: ['js', 'json', 'ts', 'json'],
        exclude: 'node_modules',
        failOnWarning: true
    })
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __git_commit_version: `"${gitCommitVersion.toString()}"`,
      __python_version: `"${pyVersion.toString()}"`,
      __build_timestamp: `"${buildTimeStamp.toString()}"`,
      'process.env.APP_TYPE': '"main"',
      __resources: `"${path.join(rootDir, 'public').replace(/\\/g, '\\\\')}"`,
    }),

    new ESLintPlugin({
        fix: true, /* 自动帮助修复 */
        extensions: ['js', 'json', 'ts', 'json'],
        exclude: 'node_modules',
        failOnWarning: true
    })
  ],
};

module.exports = (argv) => {
  return merge(webpackConfig(argv), isProduction(argv) ? prodConfig : devConfig)
};
