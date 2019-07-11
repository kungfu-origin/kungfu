'use strict'

process.env.BABEL_ENV = 'main'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')
const OptimizeJsPlugin = require("optimize-js-plugin");

let whiteListedModules = [
  'vue', 
  'element-ui', 
  'vuex', 
  'vue-router', 
  'vue-virtual-scroller', 
  'codemirror',
  "rxjs",
  "moment",
  "mime",
  "readline",
  "fast-csv"
];

let mainConfig = {
  entry: {
    main: path.join(__dirname, '../src/main/index.js')
  },
  externals: [
    ...Object.keys(dependencies || {}).filter(d => !whiteListedModules.includes(d))
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
    new webpack.NoEmitOnErrorsPlugin(),
    new OptimizeJsPlugin({
      sourceMap: false
    })
  ],
  resolve: {
    alias: {
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

/**
 * Adjust mainConfig for development settings
 */
if (process.env.NODE_ENV !== 'production') {
  mainConfig.plugins.push(
    new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../resources').replace(/\\/g, '\\\\')}"`
    })
  )
}

/**
 * Adjust mainConfig for production settings
 */
if (process.env.NODE_ENV === 'production') {
  mainConfig.plugins.push(
    new webpack.optimize.UglifyJsPlugin({
      compress: {
        warnings: false
      }
    }),
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': '"production"',
    })
  )
}


module.exports = mainConfig
