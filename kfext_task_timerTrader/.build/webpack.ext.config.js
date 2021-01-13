'use strict'

process.env.BABEL_ENV = 'main'

const path = require('path')
const webpack = require('webpack')
const OptimizeJsPlugin = require("optimize-js-plugin");


let extConfig = {
  entry: {
    index: path.join(__dirname, '../src/index.ts')
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
        include: [
          path.resolve(__dirname, '..', 'src'),
          path.resolve(__dirname, '..', '..', 'node_modules', 'kungfu-shared'),
        ]
      },
      {
        test: /\.node$/,
        use: 'node-loader'
      }
    ]
  },
  node: {
    __dirname: true,
    __filename: true 
  },
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../lib')
  },
  plugins: [
    new webpack.NoEmitOnErrorsPlugin(),
    new webpack.DefinePlugin({
      'process.env.LANG_ENV': '"en"',
      'process.env.APP_TYPE': '"ext"',
    }),
  ],
  resolve: {
    alias: {
      '@': path.join(__dirname, '../src'),
      '__gUtils': path.join(__dirname, '../../app/shared/utils'),
      '__gConfig': path.join(__dirname, '../../app/shared/config'),
      '__io': path.join(__dirname, '../../app/shared/io'),
      '__assets': path.join(__dirname, '../../app/shared/assets'),
      '__app': path.join(__dirname, '../../app'),
    },
    extensions: ['.ts', '.js', '.json', '.node']
  },
  target: 'node'
}


const { getPythonVersion } = require('../../app/.electron-kungfu/utils');
const pyVersion = getPythonVersion() || '3';

/**
 * Adjust extConfig for development settings
 */
if (process.env.NODE_ENV !== 'production') {
  extConfig.plugins.push(
    new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../../app/resources').replace(/\\/g, '\\\\')}"`,
      'process.env.NODE_ENV': '"development"',
      'process.env.APP_TYPE': '"ext"',
      'python_version': `"${pyVersion.toString()}"`,
    })
  )
}

/**
 * Adjust extConfig for production settings
 */
if (process.env.NODE_ENV === 'production') {
  extConfig.plugins.push(
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': '"production"',
      'process.env.LANG_ENV': '"en"',
      'python_version': `"${pyVersion.toString()}"`,
    }),
    new OptimizeJsPlugin({
      sourceMap: false
    })
  )
}


module.exports = extConfig
