'use strict'

process.env.BABEL_ENV = 'renderer'

const commonConfig = require('./webpack.common.config');
const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')

const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const OptimizeJsPlugin = require("optimize-js-plugin");
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const { VueLoaderPlugin } = require('vue-loader');

const { getPythonVersion, getViewsConfig } = require('./utils');
const pyVersion = getPythonVersion() || '3'
const viewsConfig = getViewsConfig();

const rootDir = path.dirname(__dirname);
const IsProduction = process.env.NODE_ENV === 'production';
const styleLoader = !IsProduction ? 'vue-style-loader' : MiniCssExtractPlugin.loader;

let rendererConfig = {
  devtool: 'eval-cheap-module-source-map',

  entry: viewsConfig.entry,

  output: {
    filename: 'js/[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(rootDir, 'dist', 'app')
  },
  
  externals: [
    ...Object.keys(dependencies || {})
  ],

  module: {
    rules: [
      { 
        test: /\.css$/,
        use: [styleLoader, "css-loader"],
      },
      {
        test: /\.scss$/,
        use: [styleLoader, "css-loader", 'sass-loader'],
      },
      {
        test: /\.ts$/,
        use: 'ts-loader',
        include: [
          path.resolve(rootDir, 'resources'),
          path.resolve(rootDir, 'src'),
          path.resolve(rootDir, 'shared'),
          path.resolve(path.dirname(rootDir), 'shared'),
        ]
      },
      {
        test: /\.js$/,
        use: 'babel-loader',
        exclude: /node_modules/
      },
      {
        test: /\.node$/,
        use: 'node-loader'
      },
      {
        test: /\.vue$/,
        use: 'vue-loader'
      },
      {
        test: /\.worker\.js$/,
        use: {
          loader: 'worker-loader',
          options: { inline: true, fallback: false }
        },
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
    __dirname: !IsProduction,
    __filename: !IsProduction
  },

  plugins: [
    ...viewsConfig.plugins,

    new MiniCssExtractPlugin({
      filename: `css/[name].css`,
    }),

    new webpack.NoEmitOnErrorsPlugin(),
    new MonacoWebpackPlugin({
      languages: [
        'python', 'cpp', 'shell', 'json', 'yaml'
      ]
    }),
    new VueLoaderPlugin()
  ],

  resolve: {
    alias: {
      '@': path.resolve(rootDir, 'src', 'renderer'),
      '__gUtils': path.resolve(rootDir, 'shared', 'utils'),
      '__gConfig': path.resolve(rootDir, 'shared', 'config'),
      '__io': path.resolve(rootDir, 'shared', 'io'),
      '__assets': path.resolve(rootDir, 'shared', 'assets')
    },
    extensions: ['.js', '.ts', '.vue', '.json', '.css', '.node']
  },

  target: 'electron-renderer',
}

/**
 * Adjust rendererConfig for development settings
 */
if (!IsProduction) {
  rendererConfig.mode = 'development';
  rendererConfig.plugins.push(
    new webpack.HotModuleReplacementPlugin(),
    new webpack.DefinePlugin({
      '__resources': `"${path.join(rootDir, 'resources').replace(/\\/g, '\\\\')}"`,
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  )
}

/**
 * Adjust rendererConfig for production settings
 */
if (IsProduction) {
  rendererConfig.mode = 'production';
  rendererConfig.devtool = 'eval'
  rendererConfig.plugins.push(
    new OptimizeJsPlugin({
      sourceMap: false
    }),
    new webpack.DefinePlugin({
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.APP_TYPE': '"renderer"',
    }),
  )
}

module.exports = rendererConfig
