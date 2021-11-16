'use strict';

process.env.BABEL_ENV = 'main';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const rootDir = path.dirname(path.dirname(__dirname));
const { isProduction } = toolkit.utils;

const webpackConfig = (argv) => merge(
  toolkit.webpack.makeConfig(argv),
  {
    entry: {
      daemon: path.join(rootDir, 'src', 'daemon', 'index.ts'),
    },
    externals: [
      'bufferutil',
      'utf-8-validate'
    ],
    resolve: {
      alias: {
        '__renderer': path.resolve(rootDir, 'src', 'renderer'),
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

module.exports = (argv) => {
  return merge(webpackConfig(argv), isProduction(argv) ? prodConfig : devConfig)
};
