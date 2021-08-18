'use strict'

process.env.BABEL_ENV = 'main'

const commonConfig = require('./webpack.common.config');
const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')
const OptimizeJsPlugin = require("optimize-js-plugin");

const { getCommitVersion, getPythonVersion } = require('./utils');
const gitCommitVersion = getCommitVersion() || 'latest'
const pyVersion = getPythonVersion() || '3'

const rootDir = path.dirname(__dirname);

let mainConfig = {
  devtool: 'eval',
  entry: {
    main: path.join(rootDir, 'src', 'main', 'index.js')
  },
  externals: [
    ...Object.keys(dependencies || {})
  ],
  module: {
    rules: [
      {
        test: /\.ts$/,
        use: 'ts-loader',
        exclude: /node_modules/
      },
      {
        test: /\.(png|jpe?g|gif|svg)(\?.*)?$/,
        use: {
          loader: 'url-loader',
          options: {
            limit: 10000,
            name: 'imgs/[name]--[folder].[ext]'
          }
        }
      },
      {
        test: /\.(mp4|webm|ogg|mp3|wav|flac|aac)(\?.*)?$/,
        loader: 'url-loader',
        options: {
          limit: 10000,
          name: 'media/[name]--[folder].[ext]'
        }
      },
      {
        test: /\.(woff2?|eot|ttf|otf)(\?.*)?$/,
        use: {
          loader: 'file-loader',
          options: {
            limit: 10000,
            name: 'fonts/[name]--[folder].[ext]'
          }
        }
      },
      ...commonConfig.module.rules
    ]
  },
  node: {
    __dirname: process.env.NODE_ENV !== 'production',
    __filename: process.env.NODE_ENV !== 'production'
  },
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(rootDir, 'dist', 'app')
  },
  plugins: [
    new webpack.NoEmitOnErrorsPlugin(),
  ],
  resolve: {
    alias: {
      '__root': rootDir,
      '@': path.resolve(rootDir, 'src', 'renderer'),
      '__gUtils': path.resolve(rootDir, 'shared', 'utils'),
      '__gConfig': path.resolve(rootDir, 'shared', 'config'),
      '__io': path.resolve(rootDir, 'shared', 'io'),
      '__assets': path.resolve(rootDir, 'shared', 'assets')
    },
    extensions: ['.js', '.ts', '.json', '.node']
  },
  target: 'electron-main'
}


/**
 * Adjust mainConfig for development settings
 */
if (process.env.NODE_ENV !== 'production') {
  mainConfig.mode = 'development';
  mainConfig.plugins.push(
    new webpack.DefinePlugin({
      'git_commit_version': `"${gitCommitVersion.toString()}"`,
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"main"',
      '__resources': `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
    }),
  )
}


/**
 * Adjust mainConfig for production settings
 */
if (process.env.NODE_ENV === 'production') {
  mainConfig.mode = 'production';
  mainConfig.devtool = 'eval'
  mainConfig.plugins.push(
    new OptimizeJsPlugin({
      sourceMap: false
    }),
    new webpack.DefinePlugin({
      'git_commit_version': `"${gitCommitVersion.toString()}"`,
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"main"',
    })
  )
}

module.exports = mainConfig
