const { IgnorePlugin, NoEmitOnErrorsPlugin } = require('webpack');
const { IsProduction } = require('./utils');

module.exports = {
  config: {
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
          test: /log4js\/lib\/appenders\/index\.js$/,
          loader: 'string-replace-loader',
          options: {
            search: 'return require(modulePath);',
            replace: 'return coreAppenders[\'file\'];',
          },
        },
        {
          test: /systeminformation\/lib\/cpu\.js$/,
          loader: 'string-replace-loader',
          options: {
            search: "require('osx-temperature-sensor')",
            replace: 'null',
          },
        },
        {
          test: /pm2\/lib\/Common\.js$/,
          loader: 'string-replace-loader',
          options: {
            search: "require.resolve(path.resolve(filename))",
            replace: '""',
          },
        },
        {
          test: /pm2\/lib\/Common\.js$/,
          loader: 'string-replace-loader',
          options: {
            search: "require(confPath)",
            replace: '{}',
          },
        },
      ],
    },
    node: {
      __dirname: !IsProduction,
      __filename: !IsProduction,
    },
    plugins: [
      new IgnorePlugin({ resourceRegExp: /deploy/, contextRegExp: /pm2-deploy$/ }),
      new IgnorePlugin({ resourceRegExp: /NPM.js/, contextRegExp: /pm2\/lib\/API\/Modules$/ }),
      new IgnorePlugin({ resourceRegExp: /TAR.js/, contextRegExp: /pm2\/lib\/API\/Modules$/ }),
      new IgnorePlugin({ resourceRegExp: /pty.js/, contextRegExp: /blessed\/lib\/widgets$/ }),
      new IgnorePlugin({ resourceRegExp: /term.js/, contextRegExp: /blessed\/lib\/widgets$/ }),
      new NoEmitOnErrorsPlugin(),
    ],
  }
};