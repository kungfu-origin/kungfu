'use strict';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const webpack = require('webpack');
const { merge } = require('webpack-merge');
const { getKungfuBuildInfo } = toolkit.utils;
const { pyVersion } = getKungfuBuildInfo();
const { VueLoaderPlugin } = require('vue-loader');
const { getThemeVariables } = require('ant-design-vue/dist/theme');
const {
  getWebpackExternals,
  getAppDir,
  getApiDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const { isProduction } = toolkit.utils;
const appDir = getAppDir();
const apiDir = getApiDir();

const tjsIncludes = [
  path.join(__dirname, '..', '..', 'src'),
  path.join(apiDir, 'src'),
  path.join(appDir, 'src'),
];

const webpackConfig = (argv) => {
  argv.passTJSLoader = true;
  return merge(toolkit.webpack.makeConfig(argv), {
    externals: getWebpackExternals(),
    entry: {
      entry: path.join(
        __dirname,
        '..',
        '..',
        'src',
        'modules',
        'entry',
        'index.ts',
      ),
      page: path.join(
        __dirname,
        '..',
        '..',
        'src',
        'modules',
        'page',
        'index.ts',
      ),
    },
    module: {
      rules: [
        {
          test: /\.(t|j)s$/,
          include: tjsIncludes,
          use: [
            {
              loader: 'babel-loader',
            },
          ],
        },
        {
          test: /\.(t|j)s$/,
          include: tjsIncludes,
          use: [
            {
              loader: 'ts-loader',
              options: {
                // 对应文件添加个.ts或.tsx后缀
                appendTsSuffixTo: [/\.vue$/],
                transpileOnly: false, // 关闭类型检测，即值进行转译
                allowTsInNodeModules: true,
                configFile: path.resolve(process.cwd(), 'tsconfig.json'),
              },
            },
          ],
        },
        {
          test: /\.css$/,
          use: ['style-loader', 'css-loader'],
        },
        {
          test: /\.less$/,
          use: [
            'style-loader',
            'css-loader',
            {
              loader: 'less-loader',
              options: {
                lessOptions: {
                  javascriptEnabled: true,
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
    plugins: [new VueLoaderPlugin()],
    output: {
      globalObject: 'this',
      filename: '[name].js',
      libraryTarget: 'commonjs2',
      path: path.join(argv.distDir, argv.distName),
    },
    experiments: {
      topLevelAwait: true,
    },
    target: 'electron-renderer',
  });
};

const prodConfig = {
  plugins: [
    new webpack.DefinePlugin({
      'process.env.APP_TYPE': '"renderer"',
      'process.env.UI_EXT_TYPE': '"component"',
      __python_version: `"${pyVersion.toString()}"`,
      __resources: '',
    }),
  ],
};

const devConfig = {
  plugins: [
    new webpack.DefinePlugin({
      'process.env.APP_TYPE': '"renderer"',
      'process.env.UI_EXT_TYPE': '"component"',
      __python_version: `"${pyVersion.toString()}"`,
      __resources: `"${path.join(appDir, 'public').replace(/\\/g, '\\\\')}"`,
    }),
  ],
};

module.exports = (argv) => {
  return merge(
    webpackConfig(argv),
    isProduction(argv) ? prodConfig : devConfig,
  );
};
