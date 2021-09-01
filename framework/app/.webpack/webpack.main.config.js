'use strict';

process.env.BABEL_ENV = 'main';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');
const { dependencies } = require('../package.json');

const rootDir = path.dirname(__dirname);
const baseConfig = toolkit.webpack.makeConfig(rootDir, 'app');
const { getPythonVersion, getCommitVersion, isProduction } = toolkit.utils;

const gitCommitVersion = getCommitVersion() || 'latest';
const pyVersion = getPythonVersion() || '3';

const webpackConfig = merge(baseConfig, {
  entry: {
    main: path.join(rootDir, 'src', 'main', 'index.js'),
  },
  externals: [...Object.keys(dependencies || {})],
  module: {
    rules: [
      {
        test: /\.(png|jpe?g|gif|svg)(\?.*)?$/,
        use: {
          loader: 'url-loader',
          options: {
            limit: 10000,
            name: 'imgs/[name]--[folder].[ext]',
          },
        },
      },
      {
        test: /\.(mp4|webm|ogg|mp3|wav|flac|aac)(\?.*)?$/,
        loader: 'url-loader',
        options: {
          limit: 10000,
          name: 'media/[name]--[folder].[ext]',
        },
      },
      {
        test: /\.(woff2?|eot|ttf|otf)(\?.*)?$/,
        use: {
          loader: 'file-loader',
          options: {
            limit: 10000,
            name: 'fonts/[name]--[folder].[ext]',
          },
        },
      },
    ],
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
