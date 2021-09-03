'use strict';

process.env.BABEL_ENV = 'renderer';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const { VueLoaderPlugin } = require('vue-loader');

const rootDir = path.dirname(__dirname);
const baseConfig = toolkit.webpack.makeConfig(rootDir, 'app');
const { getPythonVersion, getViewsConfig, isProduction } = toolkit.utils;

const pyVersion = getPythonVersion() || '3';
const viewsConfig = getViewsConfig();

const styleLoader = !isProduction() ? 'vue-style-loader' : MiniCssExtractPlugin.loader;

const webpackConfig = merge(baseConfig, {
  devtool: 'eval-cheap-module-source-map',
  entry: viewsConfig.entry,
  module: {
    rules: [
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
  plugins: [
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
    },
  },
  target: 'electron-renderer',
});

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.HotModuleReplacementPlugin(),
    new webpack.DefinePlugin({
      __resources: `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  ],
};

module.exports = merge(webpackConfig, isProduction() ? prodConfig : devConfig);
