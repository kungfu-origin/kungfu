const path = require('path');
const TerserPlugin = require('terser-webpack-plugin');
const ESLintPlugin = require('eslint-webpack-plugin');
const { isProduction, getAppDir } = require('./utils');
const appDir = getAppDir();

module.exports = {
  makeConfig: (argv) => {
    const production = isProduction(argv);
    return {
      devtool: 'eval-source-map',
      mode: production ? 'production' : 'development',
      optimization: {
        minimize: true,
        minimizer: [new TerserPlugin()],
      },
      module: {
        rules: [
          ...(argv.passTJSLoader
            ? []
            : [
                {
                  test: /\.(t|j)s$/,
                  exclude: /node_modules/,
                  use: [
                    'thread-loader',
                    {
                      loader: 'babel-loader',
                    },
                  ],
                },
                {
                  test: /\.(t|j)s$/,
                  exclude: /node_modules/,
                  use: [
                    'thread-loader',
                    {
                      loader: 'ts-loader',
                      options: {
                        // 指定特定的ts编译配置，为了区分脚本的ts配置
                        configFile: path.resolve(
                          process.cwd(),
                          'tsconfig.json',
                        ),
                        // 对应文件添加个.ts或.tsx后缀
                        appendTsSuffixTo: [/\.vue$/],
                        transpileOnly: false, // 关闭类型检测，即值进行转译
                        happyPackMode: true,
                      },
                    },
                  ],
                },
              ]),
          {
            test: /\.node$/,
            use: 'node-loader',
          },
          {
            test: /\.(m?js|node)$/,
            parser: {
              amd: false,
            },
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
                esModule: false,
              },
            },
          },
          {
            test: /\.(mp4|webm|ogg|mp3|wav|flac|aac)(\?.*)?$/,
            use: {
              loader: 'url-loader',
              options: {
                limit: 10000,
                name: 'media/[name]--[folder].[ext]',
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
        globalObject: 'this',
        filename: '[name].js',
        path: path.join(argv.distDir, argv.distName),
        library: {
          type: 'commonjs2',
        },
      },
      plugins: [
        new ESLintPlugin({
          fix: true /* 自动帮助修复 */,
          extensions: ['js', 'json', 'ts', 'json', 'css', 'less'],
          exclude: 'node_modules',
          failOnWarning: production ? false : true,
        }),
      ],
      resolve: {
        alias: {
          '@kungfu-trader/kungfu-js-api': path.resolve(
            path.dirname(
              path.dirname(require.resolve('@kungfu-trader/kungfu-js-api')),
            ),
            'src',
          ),
          '@kungfu-trader/kungfu-app': appDir,
        },

        extensions: ['.js', '.ts', '.d.ts', '.vue', '.json', '.css', '.node'],
      },
    };
  },
};
