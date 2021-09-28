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
const { getKungfuBuildInfo, getViewsConfig, isProduction } = toolkit.utils;

const { pyVersion } = getKungfuBuildInfo();

const webpackConfig = (mode) => {
  const styleLoader = isProduction(mode) ? MiniCssExtractPlugin.loader : "vue-style-loader"
  const viewsConfig = getViewsConfig(mode);

  return merge(
    toolkit.webpack.makeConfig(mode, rootDir, 'app'), 
    {
      entry: viewsConfig.entry,
      module: {
        rules: [
          {
            test: /\.css$/,
            use: [
              styleLoader,
              {
                loader: 'css-loader',
                options: {
                  esModule: false
                }
              }],
          },
          {
            test: /\.scss$/,
            use: [
              styleLoader,
              {
                loader: 'css-loader',
                options: {
                  esModule: false
                }
              },
              'sass-loader'
            ],
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
            test: /\.html$/,
            use: "html-loader"
          }
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
  })
};

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
    new webpack.DefinePlugin({
      __resources: `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
      python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  ],
};

module.exports = (env, argv) => {
  const mode = argv.mode;
  return merge(webpackConfig(mode), isProduction(mode) ? prodConfig : devConfig)
};