'use strict';

process.env.BABEL_ENV = 'main';

const toolchain = require('@kungfu-trader/kungfu-toolchain');
const path = require('path');
const webpack = require('webpack');
const { dependencies } = require('../package.json');

const commonConfig = toolchain.webpack.config;
const { getPythonVersion, getCommitVersion, IsProduction } = toolchain.utils;

const pyVersion = getPythonVersion() || '3';

const nodeModules = {};
Object.keys(dependencies || {})
  .filter(function(x) {
    return ['.bin'].indexOf(x) === -1;
  })
  .forEach(function(mod) {
    nodeModules[mod] = 'commonjs ' + mod;
  });

let cliConfig = {
  entry: {
    index: path.join(__dirname, '../src/index.ts'),
  },
  externals: {
    ...nodeModules,
  },
  module: {
    rules: [...commonConfig.module.rules],
  },
  node: commonConfig.node,
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.resolve(__dirname, '../dist/cli'),
  },
  plugins: [...commonConfig.plugins],
  resolve: {
    alias: {
      '@kungfu-trader/kungfu-cli': path.resolve(__dirname, '../src'),
      '@kungfu-trader/kungfu-uicc': path.dirname(require.resolve('@kungfu-trader/kungfu-uicc')),
    },
    extensions: ['.ts', '.js', '.json', '.node'],
  },
  target: 'node',
};

/**
 * Adjust cliConfig for development settings
 */
if (!IsProduction) {
  cliConfig.mode = 'development';
  cliConfig.plugins.push(
    new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../../app/resources').replace(/\\/g, '\\\\')}"`,
      'process.env.NODE_ENV': '"development"',
      'process.env.APP_TYPE': '"cli"',
      'process.env.LANG_ENV': '"en"',
      'python_version': `"${pyVersion.toString()}"`,
    }),
  );
}

/**
 * Adjust cliConfig for production settings
 */
if (IsProduction) {
  cliConfig.mode = 'production';
  cliConfig.plugins.push(
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': '"production"',
      'process.env.APP_TYPE': '"cli"',
      'process.env.LANG_ENV': '"en"',
      'python_version': `"${pyVersion.toString()}"`,
    }),
  );
}

module.exports = cliConfig;
