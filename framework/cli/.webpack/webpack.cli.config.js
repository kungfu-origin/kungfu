'use strict';

process.env.BABEL_ENV = 'main';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');
const { dependencies } = require('../package.json');

const appDir = path.dirname(require.resolve('@kungfu-trader/kungfu-app/package.json'));
const rootDir = path.dirname(__dirname);
const baseConfig = toolkit.webpack.makeConfig(rootDir, 'cli');
const { getPythonVersion, isProduction } = toolkit.utils;

const pyVersion = getPythonVersion() || '3';

const nodeModules = {};
Object.keys(dependencies || {})
  .filter(function(x) {
    return ['.bin'].indexOf(x) === -1;
  })
  .forEach(function(mod) {
    nodeModules[mod] = 'commonjs ' + mod;
  });

const webpackConfig = merge(baseConfig, {
  entry: {
    index: path.join(rootDir, 'src', 'index.ts'),
  },
  externals: {
    ...nodeModules,
  },
  resolve: {
    alias: {
      '@kungfu-trader/kungfu-cli': path.resolve(rootDir, 'src'),
    },
    extensions: ['.ts', '.js', '.json', '.node'],
  },
  target: 'node',
});

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': '"production"',
      'process.env.APP_TYPE': '"cli"',
      'process.env.LANG_ENV': '"en"',
      'python_version': `"${pyVersion.toString()}"`,
    }),
  ]
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      '__resources': `"${path.join(appDir, 'resources').replace(/\\/g, '\\\\')}"`,
      'process.env.NODE_ENV': '"development"',
      'process.env.APP_TYPE': '"cli"',
      'process.env.LANG_ENV': '"en"',
      'python_version': `"${pyVersion.toString()}"`,
    }),
  ]
};

module.exports = merge(webpackConfig, isProduction() ? prodConfig : devConfig);
