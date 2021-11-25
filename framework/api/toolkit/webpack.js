const { NoEmitOnErrorsPlugin } = require('webpack');
const { isProduction } = require('./utils');
const path = require("path");


module.exports = {
  makeConfig: (argv) => {
    const production = isProduction(argv.mode);
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
        path: path.join(argv.distDir, argv.distName),
      },
      plugins: [
        new NoEmitOnErrorsPlugin(),
      ],
      resolve: {
        alias: {
          '@kungfu-trader/kungfu-uicc': path.dirname(path.resolve(require.resolve('@kungfu-trader/kungfu-uicc'))),
          '@kungfu-trader/kungfu-core': path.dirname(path.resolve(require.resolve('@kungfu-trader/kungfu-core'))),
          '@kungfu-trader/kungfu-js-api': path.join(path.dirname(path.dirname(require.resolve('@kungfu-trader/kungfu-js-api'))), 'src'),
        },

        extensions: ['.js', '.ts', '.vue', '.json', '.css', '.node'],
      },
    };
  },
};
