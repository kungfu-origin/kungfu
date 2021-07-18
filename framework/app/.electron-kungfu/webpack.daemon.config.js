'use strict'

process.env.BABEL_ENV = 'main'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')
const OptimizeJsPlugin = require("optimize-js-plugin");

let daemonConfig = {
  entry: {
    daemon: path.join(__dirname, '../src/daemon/index.ts')
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
    new webpack.NoEmitOnErrorsPlugin(),
  ],
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
  target: 'node'
}

/**
 * Adjust daemonConfig for development settings
 */
if (process.env.NODE_ENV !== 'production') {
  daemonConfig.plugins.push(
    new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../resources').replace(/\\/g, '\\\\')}"`,
      'process.env.NODE_ENV': '"development"',
      'process.env.APP_TYPE': '"daemon"',
    }),
  )
}

/**
 * Adjust daemonConfig for production settings
 */
if (process.env.NODE_ENV === 'production') {
  daemonConfig.devtool = ''
  daemonConfig.plugins.push(
    new OptimizeJsPlugin({
      sourceMap: false
    }),
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': '"production"',
      'process.env.APP_TYPE': '"daemon"',
    })
  )
}

module.exports = daemonConfig
