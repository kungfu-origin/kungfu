const { IgnorePlugin, NoEmitOnErrorsPlugin } = require('webpack');
const { isProduction } = require('./utils');
const path = require("path");

module.exports = {
  makeConfig: (rootDir, distName) => {
    return {
      devtool: 'eval',
      mode: isProduction() ? 'production' : 'development',
      module: {
        rules: [
          {
            test: /\.js$/,
            use: 'babel-loader',
            exclude: /node_modules/,
          },
          {
            test: /\.ts$/,
            use: 'ts-loader',
            exclude: /node_modules/,
          },
          {
            test: /\.node$/,
            use: 'node-loader',
          },
          {
            test: /\.(m?js|node)$/,
            parser: { amd: false },
            use: {
              loader: '@vercel/webpack-asset-relocator-loader',
              options: {
                outputAssetBase: 'native_modules',
                production: isProduction(),
              },
            },
          },
          {
            test: /pm2\/lib\/Client\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: "'node'",
              replace: "'kfc'",
            },
          },
          {
            test: /pm2\/lib\/Client\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: 'module.filename',
              replace: '__filename',
            },
          },
        ],
      },
      node: {
        __dirname: !isProduction(),
        __filename: !isProduction(),
      },
      output: {
        filename: '[name].js',
        libraryTarget: 'commonjs2',
        path: path.join(rootDir, 'dist', distName),
      },
      plugins: [
        new IgnorePlugin({ resourceRegExp: /deploy/, contextRegExp: /pm2-deploy$/ }),
        new IgnorePlugin({ resourceRegExp: /pty.js/, contextRegExp: /blessed\/lib\/widgets$/ }),
        new IgnorePlugin({ resourceRegExp: /term.js/, contextRegExp: /blessed\/lib\/widgets$/ }),
        new NoEmitOnErrorsPlugin(),
      ],
      resolve: {
        alias: {
          '@kungfu-trader/kungfu-uicc': path.dirname(require.resolve('@kungfu-trader/kungfu-uicc')),
        },
        extensions: ['.js', '.ts', '.vue', '.json', '.css', '.node'],
      },
    };
  },
};
