'use strict';

process.env.BABEL_ENV = 'main';

const toolchain = require('@kungfu-trader/kungfu-toolchain');
const path = require('path');
const webpack = require('webpack');
const { dependencies } = require('../package.json');

const commonConfig = toolchain.webpack.config;
const { getPythonVersion, getCommitVersion, IsProduction } = toolchain.utils;

const gitCommitVersion = getCommitVersion() || 'latest';
const pyVersion = getPythonVersion() || '3';

const rootDir = path.dirname(__dirname);

let mainConfig = {
  devtool: 'eval',
  entry: {
    main: path.join(rootDir, 'src', 'main', 'index.js'),
  },
  externals: [...Object.keys(dependencies || {})],
  module: {
    rules: [
      ...commonConfig.module.rules,
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
  node: commonConfig.node,
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(rootDir, 'dist', 'app'),
  },
  plugins: [...commonConfig.plugins],
  resolve: {
    alias: {
      '@': path.resolve(rootDir, 'src', 'renderer'),
      '@kungfu-trader/kungfu-uicc': path.dirname(require.resolve('@kungfu-trader/kungfu-uicc')),
    },
    extensions: ['.js', '.ts', '.json', '.node'],
  },
  target: 'electron-main',
};

/**
 * Adjust mainConfig for development settings
 */
if (!IsProduction) {
  mainConfig.mode = 'development';
  mainConfig.plugins.push(
    new webpack.DefinePlugin({
      git_commit_version: `"${gitCommitVersion.toString()}"`,
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"main"',
      __resources: `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
    }),
  );
}

/**
 * Adjust mainConfig for production settings
 */
if (IsProduction) {
  mainConfig.mode = 'production';
  mainConfig.devtool = 'eval';
  mainConfig.plugins.push(
    new webpack.DefinePlugin({
      git_commit_version: `"${gitCommitVersion.toString()}"`,
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"main"',
    }),
  );
}

module.exports = mainConfig;
