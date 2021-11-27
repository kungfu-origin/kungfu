'use strict';

process.env.BABEL_ENV = 'renderer';

const toolkit = require('@kungfu-trader/kungfu-uicc/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');

const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const { VueLoaderPlugin } = require('vue-loader');

const rootDir = path.dirname(path.dirname(__dirname));
const { getKungfuBuildInfo, getViewsConfig, isProduction } = toolkit.utils;

const { pyVersion } = getKungfuBuildInfo();

const webpackConfig = (argv) => {
  const styleLoader = isProduction(argv) ? MiniCssExtractPlugin.loader : "vue-style-loader"
  const viewsConfig = getViewsConfig(argv);

  return merge(
    toolkit.webpack.makeConfig(argv),
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
        '__renderer': path.resolve(rootDir, 'src', 'renderer'),
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

module.exports = (argv) => {
  return merge(webpackConfig(argv), isProduction(argv) ? prodConfig : devConfig)
};