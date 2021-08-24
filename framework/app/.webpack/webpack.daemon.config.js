'use strict';

process.env.BABEL_ENV = 'main';

const toolchain = require('@kungfu-trader/kungfu-toolchain');
const path = require('path');
const webpack = require('webpack');
const { dependencies } = require('../package.json');

const commonConfig = toolchain.webpack.config;
const { IsProduction } = toolchain.utils;

let daemonConfig = {
  devtool: 'eval',
  entry: {
    daemon: path.join(__dirname, '../src/daemon/index.ts'),
  },
  externals: [...Object.keys(dependencies || {})],
  module: {
    rules: [...commonConfig.module.rules],
  },
  node: commonConfig.node,
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../dist/app'),
  },
  plugins: [...commonConfig.plugins],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, '../src/renderer'),
      '@kungfu-trader/kungfu-uicc': path.dirname(require.resolve('@kungfu-trader/kungfu-uicc')),
    },
    extensions: ['.js', '.ts', '.json', '.node'],
  },
  target: 'node',
};

/**
 * Adjust daemonConfig for development settings
 */
if (!IsProduction) {
  daemonConfig.mode = 'development';
  daemonConfig.plugins.push(
    new webpack.DefinePlugin({
      __resources: `"${path.join(__dirname, '../resources').replace(/\\/g, '\\\\')}"`,
      'process.env.APP_TYPE': '"daemon"',
    }),
  );
}

/**
 * Adjust daemonConfig for production settings
 */
if (IsProduction) {
  daemonConfig.mode = 'production';
  daemonConfig.devtool = 'eval';
  daemonConfig.plugins.push(
    new webpack.DefinePlugin({
      'process.env.APP_TYPE': '"daemon"',
    }),
  );
}

module.exports = daemonConfig;
