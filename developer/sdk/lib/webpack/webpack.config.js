'use strict';

const toolkit = require('@kungfu-trader/kungfu-js-api/toolkit');
const path = require('path');
const { merge } = require('webpack-merge');
const {
  getWebpackExternals,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const webpackConfig = (argv) => {
  argv.passTJSLoader = true;
  return merge(toolkit.webpack.makeConfig(argv), {
    externals: getWebpackExternals(),
    mode: 'production',
    entry: {
      kfs: path.join(__dirname, '..', '..', 'src', 'kfs.js'),
    },
    module: {
      rules: [
        {
          test: /\.(t|j)s$/,
          include: [path.join(__dirname, '..', '..', 'src')],
          use: [
            {
              loader: 'babel-loader',
            },
          ],
        },
      ],
    },
    resolve: {
      alias: {
        '@kungfu-trader/kungfu-js-api': path.join(
          require.resolve('@kungfu-trader/kungfu-js-api'),
          '../../',
        ),
      },
    },
    target: 'node',
  });
};

module.exports = (argv) => {
  return webpackConfig(argv);
};
