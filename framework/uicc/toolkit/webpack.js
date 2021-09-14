const { IgnorePlugin, NoEmitOnErrorsPlugin } = require('webpack');
const nodeExternals = require('webpack-node-externals');
const { isProduction } = require('./utils');
const path = require("path");

module.exports = {
  makeConfig: (rootDir, distName) => {
    const production = isProduction();
    return {
      devtool: 'eval',
      externals: [
        nodeExternals({
          allowlist: [/^@kungfu-trader/],
          modulesDir: path.resolve(rootDir, 'node_modules')
        })
      ],
      mode: production ? 'production' : 'development',
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
            test: /@pm2\/agent\/src\/InteractorClient\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: 'main.filename',
              replace: "resolve('pm2/bin/pm2')",
            },
          },
          {
            test: /@pm2\/agent\/src\/InteractorClient\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: 'path.dirname(module.filename)',
              replace:
                production ? 'path.resolve(__dirname, "..", "..", "node_modules", "@pm2", "agent", "src")' : '__dirname',
            },
          },
          {
            test: /pm2\/lib\/Client\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: "interpreter = 'node'",
              replace: "interpreter = which('kfc')",
            },
          },
          {
            test: /pm2\/lib\/Client\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: "which('node')",
              replace: "which('kfc')",
            },
          },
          {
            test: /pm2\/lib\/Client\.js$/,
            loader: 'string-replace-loader',
            options: {
              search: 'path.dirname(module.filename)',
              replace:
                production ? 'path.resolve(__dirname, "..", "..", "node_modules", "pm2", "lib")' : '__dirname',
            },
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
        ],
      },
      node: {
        __dirname: !production,
        __filename: !production,
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
