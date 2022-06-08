'use strict';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');

const webpack = require('webpack');
const { merge } = require('webpack-merge');

const { VueLoaderPlugin } = require('vue-loader');
const { getThemeVariables } = require('ant-design-vue/dist/theme');
const {
  getWebpackExternals,
  getAppDir,
  getCoreDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const CopyPlugin = require('copy-webpack-plugin');
const fse = require('fs-extra');

const appDir = getAppDir();
const { getKungfuBuildInfo, getPagesConfig, isProduction } = toolkit.utils;
const { pyVersion } = getKungfuBuildInfo();
const publicDir = path.join(appDir, 'public');

const webpackConfig = (argv) => {
  const pagesConfig = getPagesConfig(argv);
  fse.removeSync(path.join(publicDir, 'python'));

  return merge(toolkit.webpack.makeConfig(argv), {
    externals: getWebpackExternals(),
    entry: pagesConfig.entry,
    module: {
      rules: [
        {
          test: /\.css$/,
          use: ['style-loader', 'postcss-loader', 'css-loader'],
        },
        {
          test: /\.less$/,
          use: [
            'style-loader',
            'postcss-loader',
            'css-loader',
            {
              loader: 'less-loader',
              options: {
                lessOptions: {
                  javascriptEnabled: true,
                  modifyVars: {
                    ...getThemeVariables({
                      dark: true,
                      compact: true,
                    }),
                    'font-size-base': '12px', // major text font size
                    'primary-color': '#FAAD14',
                    'red-base': '#d65453',
                    'green-base': '#17b07f',
                    'red2-base': '#f21717',
                    'green2-base': '#15de9e',
                  },
                },
              },
            },
          ],
        },
        {
          test: /\.vue$/,
          use: 'vue-loader',
        },
        {
          test: /\.worker\.ts$/,
          use: {
            loader: 'worker-loader',
            options: { inline: true, fallback: false },
          },
          exclude: /node_modules/,
        },
        {
          test: /\.html$/,
          use: 'html-loader',
        },
      ],
    },
    plugins: [
      ...pagesConfig.plugins,
      new VueLoaderPlugin(),
      new MonacoWebpackPlugin({
        languages: ['python', 'yaml', 'json'],
      }),
    ],
    target: 'electron-renderer',
  });
};

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __VUE_OPTIONS_API__: true,
      __VUE_PROD_DEVTOOLS__: true,
      __resources: '',
      __python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
      'process.env.LANG_ENV': '"zh-CN"',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __VUE_OPTIONS_API__: true,
      __VUE_PROD_DEVTOOLS__: true,
      __resources: `"${publicDir.replace(/\\/g, '\\\\')}"`,
      __python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
      'process.env.LANG_ENV': '"zh-CN"',
    }),
    new CopyPlugin({
      patterns: [
        {
          from: path.join(
            path.resolve(getCoreDir(), 'build', 'python', 'dist'),
            '*.whl',
          ),
          to: path.join(publicDir, 'python'),
          context: path.resolve(getCoreDir(), 'build', 'python', 'dist'),
        },
      ],
    }),
  ],
};

module.exports = (argv) => {
  return merge(
    webpackConfig(argv),
    isProduction(argv) ? prodConfig : devConfig,
  );
};
