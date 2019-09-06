'use strict'

process.env.BABEL_ENV = 'main'

const path = require('path')
const webpack = require('webpack')
const OptimizeJsPlugin = require("optimize-js-plugin");
const { dependencies } = require('../package.json');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');


const nodeModules = {};
Object.keys(dependencies || {})
  .filter(function(x) {
    return ['.bin'].indexOf(x) === -1 
  })
  .forEach(function(mod) {
    nodeModules[mod] = 'commonjs ' + mod;
  });

nodeModules['kungfu-core'] = 'commonjs kungfu-core';

let cliConfig = {
  entry: {
    index: path.join(__dirname, '../src/index.ts')
  },
  externals: {
    ...nodeModules,
  },
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
    path: path.join(__dirname, '../dist/cli')
  },
  plugins: [
    new webpack.NoEmitOnErrorsPlugin()
  ],
  resolve: {
    alias: {
      '@': path.join(__dirname, '../src'),
      '__gUtils': path.join(__dirname, '../../app/shared/utils'),
      '__gConfig': path.join(__dirname, '../../app/shared/config'),
      '__io': path.join(__dirname, '../../app/shared/io'),
      '__assets': path.join(__dirname, '../../app/shared/assets'),
    },
    extensions: ['.ts', '.js', '.json', '.node']
  },
  target: 'electron-main'
}
console.log(path.join(__dirname, '../src'))
/**
 * Adjust cliConfig for development settings
 */
if (process.env.NODE_ENV !== 'production') {
  cliConfig.plugins.push(
    new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../../app/resources').replace(/\\/g, '\\\\')}"`
    })
  )
}

/**
 * Adjust cliConfig for production settings
 */
if (process.env.NODE_ENV === 'production') {
  cliConfig.plugins.push(
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': '"production"',
    }),
    new OptimizeJsPlugin({
      sourceMap: false
    })
  )
}


module.exports = cliConfig
