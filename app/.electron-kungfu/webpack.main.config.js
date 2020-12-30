'use strict'

process.env.BABEL_ENV = 'main'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')
const { getCommitVersion, getPythonVersion } = require('./utils');
const gitCommitVersion = getCommitVersion() || 'latest'
const pyVersion = getPythonVersion() || '3'

const isProd = process.env.NODE_ENV === 'production';

console.log('NODE_ENV', process.env.NODE_ENV)
console.log('gitCommitVersion', gitCommitVersion)
console.log('pyVersion', pyVersion)

module.exports = {
  
  devtool: 'eval',
  entry: {
    main: path.join(__dirname, '../src/main/index.js')
  },
  externals: [
    ...Object.keys(dependencies || {})
  ],
  module: {
    rules: [
      {
        test: /\.js$/,
        use: 'babel-loader',
        exclude: /node_modules/
      },
      {
        test: /\.ts$/,
        use: 'ts-loader',
        exclude: /node_modules/
      },
      {
        test: /\.node$/,
        use: 'node-loader'
      },
      {
        test: /\.(png|jpe?g|gif|svg)(\?.*)?$/,
        use: {
          loader: 'url-loader',
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
          loader: 'url-loader'
        }
      }
    ]
  },
  node: {
    __dirname: process.env.NODE_ENV !== 'production',
    __filename: process.env.NODE_ENV !== 'production'
  },
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../dist/app')
  },

  plugins: [

    new webpack.DefinePlugin({
      'git_commit_version': `"${gitCommitVersion.toString()}"`,
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.NODE_ENV': isProd ? 'development' : '"development"',
      'process.env.APP_TYPE': '"main"',
    }),

    !isProd ? new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../resources').replace(/\\/g, '\\\\')}"`,
    }): null

  ].filter(p => !!p),

  resolve: {
    alias: {
      '__root': path.join(__dirname, '..'),
      '@': path.join(__dirname, '../src/renderer'),
      '__gUtils': path.join(__dirname, '../shared/utils'),
      '__gConfig': path.join(__dirname, '../shared/config'),
      '__io': path.join(__dirname, '../shared/io'),
      '__assets': path.join(__dirname, '../shared/assets')
    },
    extensions: ['.js', '.ts', '.json', '.node']
  },
  target: 'electron-main'
}