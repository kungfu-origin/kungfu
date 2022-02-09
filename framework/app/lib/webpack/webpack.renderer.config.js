'use strict';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');

const webpack = require('webpack');
const { merge } = require('webpack-merge');

const { VueLoaderPlugin } = require('vue-loader');
const { getThemeVariables } = require('ant-design-vue/dist/theme');
const {
  getWebpackExternals,
  getAppDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const appDir = getAppDir();
const { getKungfuBuildInfo, getPagesConfig, isProduction } = toolkit.utils;
const { pyVersion } = getKungfuBuildInfo();

const webpackConfig = (argv) => {
  const pagesConfig = getPagesConfig(argv);
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
                  modifyVars: {
                    ...getThemeVariables({
                      dark: true,
                    }),
                    'font-size-base': '12px', // major text font size
                    'primary-color': '#FAAD14',
                    'red-base': '#d65453',
                    'green-base': '#17b07f',
                    'red2-base': '#f21717',
                    'green2-base': '#15de9e',
                  },
                  javascriptEnabled: true,
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
    plugins: [...pagesConfig.plugins, new VueLoaderPlugin()],
    resolve: {
      alias: {
        '@root': appDir,
        '@renderer': path.resolve(appDir, 'src', 'renderer'),
      },
    },
    target: 'electron-renderer',
  });
};

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __VUE_OPTIONS_API__: true,
      __VUE_PROD_DEVTOOLS__: true,
      __python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
      __resources: '',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      __VUE_OPTIONS_API__: true,
      __VUE_PROD_DEVTOOLS__: true,
      __resources: `"${path.join(appDir, 'public').replace(/\\/g, '\\\\')}"`,
      __python_version: `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  ],
};

module.exports = (argv) => {
  return merge(
    webpackConfig(argv),
    isProduction(argv) ? prodConfig : devConfig,
  );
};
