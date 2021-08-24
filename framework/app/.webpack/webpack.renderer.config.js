'use strict';

process.env.BABEL_ENV = 'renderer';

const toolchain = require('@kungfu-trader/kungfu-toolchain');
const path = require('path');
const webpack = require('webpack');
const { dependencies } = require('../package.json');

const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const { VueLoaderPlugin } = require('vue-loader');

const commonConfig = toolchain.webpack.config;
const { getPythonVersion, getViewsConfig, IsProduction } = toolchain.utils;

const pyVersion = getPythonVersion() || '3';
const viewsConfig = getViewsConfig();

const rootDir = path.dirname(__dirname);
const styleLoader = !IsProduction ? 'vue-style-loader' : MiniCssExtractPlugin.loader;

let rendererConfig = {
  devtool: 'eval-cheap-module-source-map',
  entry: viewsConfig.entry,
  output: {
    filename: 'js/[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(rootDir, 'dist', 'app'),
  },
  externals: [...Object.keys(dependencies || {})],
  module: {
    rules: [
      ...commonConfig.module.rules,
      {
        test: /\.css$/,
        use: [styleLoader, 'css-loader'],
      },
      {
        test: /\.scss$/,
        use: [styleLoader, 'css-loader', 'sass-loader'],
      },
      {
        test: /\.vue$/,
        use: 'vue-loader',
      },
      {
        test: /\.worker\.js$/,
        use: {
          loader: 'worker-loader',
          options: { inline: true, fallback: false },
        },
        exclude: /node_modules/,
      },
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
  plugins: [
    ...commonConfig.plugins,
    ...viewsConfig.plugins,

    new MiniCssExtractPlugin({
      filename: `css/[name].css`,
    }),
    new MonacoWebpackPlugin({
      languages: ['python', 'cpp', 'shell', 'json', 'yaml'],
    }),
    new VueLoaderPlugin(),
  ],
  resolve: {
    alias: {
      '@': path.resolve(rootDir, 'src', 'renderer'),
      '@kungfu-trader/kungfu-app': path.resolve(rootDir, 'src', 'renderer'),
      '@kungfu-trader/kungfu-uicc': path.dirname(require.resolve('@kungfu-trader/kungfu-uicc')),
    },
    extensions: ['.js', '.ts', '.vue', '.json', '.css', '.node'],
  },
  target: 'electron-renderer',
};

/**
 * Adjust rendererConfig for development settings
 */
if (!IsProduction) {
  rendererConfig.mode = 'development';
  rendererConfig.plugins.push(
    new webpack.HotModuleReplacementPlugin(),
    new webpack.DefinePlugin({
      __resources: `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  );
}

/**
 * Adjust rendererConfig for production settings
 */
if (IsProduction) {
  rendererConfig.mode = 'production';
  rendererConfig.devtool = 'eval';
  rendererConfig.plugins.push(
    new webpack.DefinePlugin({
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  );
}

module.exports = rendererConfig;
