const { IgnorePlugin, NoEmitOnErrorsPlugin } = require('webpack');
const nodeExternals = require('webpack-node-externals');
const { isProduction } = require('./utils');
const path = require("path");
const { dependencies } = require('../package.json')

module.exports = {
  makeConfig: (mode, rootDir, distName) => {

    const production = isProduction(mode);
    return {
      devtool: 'eval-source-map',

      externals: [
        "pm2",
        "iconv-lite"
      ],
      mode: production ? 'production' : 'development',
      stats: {
        // logging: "warn",
        errorDetails: true,
      },
      infrastructureLogging: {
        // level: 'error',
      },
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
          // {
          //   test: /@pm2.*InteractorClient\.js$/,
          //   loader: 'string-replace-loader',
          //   options: {
          //     search: 'main.filename',
          //     replace: "resolve('pm2/bin/pm2')",
          //   },
          // },
          // {
          //   test: /@pm2.*InteractorClient\.js$/,
          //   loader: 'string-replace-loader',
          //   options: {
          //     search: 'path.dirname(module.filename)',
          //     replace:
          //       production ? 'path.resolve(__dirname, "..", "..", "node_modules", "@pm2", "agent", "src")' : '__dirname',
          //   },
          // },
          // {
          //   test: /pm2.*Client\.js$/,
          //   loader: 'string-replace-loader',
          //   options: {
          //     search: "interpreter = 'node'",
          //     replace: "interpreter = which('kfc')",
          //   },
          // },
          // {
          //   test: /pm2.*Client\.js$/,
          //   loader: 'string-replace-loader',
          //   options: {
          //     search: "which('node')",
          //     replace: "which('kfc')",
          //   },
          // },
          // {
          //   test: /pm2.*Client\.js$/,
          //   loader: 'string-replace-loader',
          //   options: {
          //     search: 'path.dirname(module.filename)',
          //     replace:
          //       production ? 'path.resolve(__dirname, "..", "..", "node_modules", "pm2", "lib")' : '__dirname',
          //   },
          // },
          {
            test: /\.(m?js|node)$/,
            parser: { amd: false },
            use: {
              loader: '@vercel/webpack-asset-relocator-loader',
              options: {
                outputAssetBase: 'native_modules',
                production: production,
              },
            },
          },
          {
            test: /\.(png|jpe?g|gif|svg)(\?.*)?$/,
            use: {
              loader: 'url-loader',
              options: {
                limit: 10000,
                name: 'imgs/[name]--[folder].[ext]',
                esModule: false
              },
            },
          },
          {
            test: /\.(mp4|webm|ogg|mp3|wav|flac|aac)(\?.*)?$/,
            use: {
              loader: "url-loder",
              options: {
                limit: 10000,
                name: 'media/[name]--[folder].[ext]',
                esModule: false
              },
            }
          },
          {
            test: /\.(woff2?|eot|ttf|otf)(\?.*)?$/,
            use: {
              loader: 'file-loader',
              options: {
                limit: 10000,
                name: 'fonts/[name]--[folder].[ext]',
                esModule: false,
              },
            },
          },
        ],
      },
      node: {
        __dirname: !production,
        __filename: !production,
      },
      output: {
        globalObject: "this",
        filename: '[name].js',
        libraryTarget: 'commonjs2',
        path: path.join(rootDir, 'dist', distName),
      },
      plugins: [
        // new IgnorePlugin({ resourceRegExp: /deploy/, contextRegExp: /pm2-deploy$/ }),
        // new IgnorePlugin({ resourceRegExp: /pty.js/, contextRegExp: /blessed.*widgets$/ }),
        // new IgnorePlugin({ resourceRegExp: /term.js/, contextRegExp: /blessed.*widgets$/ }),
        new NoEmitOnErrorsPlugin(),
      ],
      resolve: {
        alias: {
          '__renderer': path.resolve(rootDir, 'src', 'renderer'),
          '@kungfu-trader/kungfu-app': path.resolve(rootDir, 'src', 'renderer'),
          '@kungfu-trader/kungfu-uicc': path.dirname(path.resolve(require.resolve('@kungfu-trader/kungfu-uicc'))),
          '@kungfu-trader/kungfu-core': path.dirname(path.resolve(require.resolve('@kungfu-trader/kungfu-core'), '../../')),
        },
        extensions: ['.js', '.ts', '.vue', '.json', '.css', '.node'],
      },
    };
  },
};
