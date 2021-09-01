'use strict';

process.env.BABEL_ENV = 'main';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');
const { dependencies } = require('../package.json');

const rootDir = path.dirname(__dirname);
const baseConfig = toolkit.webpack.makeConfig(rootDir, 'app');
const { isProduction } = toolkit.utils;

const webpackConfig = merge(baseConfig, {
  entry: {
    daemon: path.join(rootDir, 'src', 'daemon', 'index.ts'),
  },
  externals: [...Object.keys(dependencies || {})],
  resolve: {
    alias: {
      '@': path.resolve(rootDir, 'src', 'renderer'),
    },
  },
  target: 'node',
});

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      'process.env.APP_TYPE': '"daemon"',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __resources: `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
      'process.env.APP_TYPE': '"daemon"',
    }),
  ],
};

module.exports = merge(webpackConfig, isProduction() ? prodConfig : devConfig);
