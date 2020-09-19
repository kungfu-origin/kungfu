'use strict'

process.env.BABEL_ENV = 'renderer'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')

const ExtractTextPlugin = require('extract-text-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const OptimizeJsPlugin = require("optimize-js-plugin");
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');
const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;


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

let rendererConfig = {
  devtool: '#cheap-module-eval-source-map',

  entry: {
    index: path.join(__dirname, '../src/renderer/views/index/main.js'),
    code: path.join(__dirname, '../src/renderer/views/code/main.js'),
    makeOrder: path.join(__dirname, '../src/renderer/views/makeOrder/main.js'),
  },

  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../dist/app')
  },
  
  externals: [
    ...Object.keys(dependencies || {}).filter(d => !whiteListedModules.includes(d))
  ],

  module: {
    rules: [
      {
        test: /\.css$/,
        use: ExtractTextPlugin.extract({
          fallback: 'style-loader',
          use: 'css-loader'
        })
      },
      {
        test: /\.scss$/,
        loaders: [{
            loader: "style-loader" // 将 JS 字符串生成为 style 节点
        }, {
            loader: "css-loader" // 将 CSS 转化成 CommonJS 模块
        }, {
            loader: "sass-loader" // 将 Sass 编译成 CSS
        }]
      },
      {
        test: /\.html$/,
        use: 'vue-html-loader'
      },
      {
        test: /\.ts$/,
        use: 'ts-loader',
        exclude: /node_modules/
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
        use: {
          loader: 'vue-loader',
          options: {
            extractCSS: process.env.NODE_ENV === 'production',
            loaders: {
              sass: 'vue-style-loader!css-loader!sass-loader?indentedSyntax=1',
              scss: 'vue-style-loader!css-loader!sass-loader'
            }
          }
        }
      },
      {
        test: /\.(png|jpe?g|gif|svg)(\?.*)?$/,
        use: {
          loader: 'url-loader',
          query: {
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
          loader: 'url-loader',
          query: {
            limit: 10000,
            name: 'fonts/[name]--[folder].[ext]'
          }
        }
      }
    ]
  },

  node: {
    __dirname: process.env.NODE_ENV !== 'production',
    __filename: process.env.NODE_ENV !== 'production'
  },

  plugins: [
    new ExtractTextPlugin('styles.css'),
    new HtmlWebpackPlugin({
      filename: 'index.html',
      template: path.resolve(__dirname, '../src/index.ejs'),
      minify: {
        collapseWhitespace: true,
        removeAttributeQuotes: true,
        removeComments: true
      },
      chunks: ['index'],
      nodeModules: process.env.NODE_ENV !== 'production'
        ? path.resolve(__dirname, '../node_modules')
        : false
    }),
    new HtmlWebpackPlugin({
      filename: 'code.html',
      template: path.resolve(__dirname, '../src/index.ejs'),
      minify: {
        collapseWhitespace: true,
        removeAttributeQuotes: true,
        removeComments: true
      },
      chunks: ['code'],
      nodeModules: process.env.NODE_ENV !== 'production'
        ? path.resolve(__dirname, '../node_modules')
        : false
    }),
    new HtmlWebpackPlugin({
      filename: 'makeOrder.html',
      template: path.resolve(__dirname, '../src/index.ejs'),
      minify: {
        collapseWhitespace: true,
        removeAttributeQuotes: true,
        removeComments: true
      },
      chunks: ['makeOrder'],
      nodeModules: process.env.NODE_ENV !== 'production'
        ? path.resolve(__dirname, '../node_modules')
        : false
    }),

    new webpack.NoEmitOnErrorsPlugin(),
    new MonacoWebpackPlugin({
      languages: [
        'python', 'cpp', 'shell', 'json', 'yaml'
      ]
    })
  ],

  resolve: {
    alias: {
      '@': path.join(__dirname, '../src/renderer'),
      '__gUtils': path.join(__dirname, '../shared/utils'),
      '__gConfig': path.join(__dirname, '../shared/config'),
      '__io': path.join(__dirname, '../shared/io'),
      '__assets': path.join(__dirname, '../shared/assets'),
      'vue$': 'vue/dist/vue.esm.js'
    },
    extensions: ['.js', '.ts', '.vue', '.json', '.css', '.node']
  },

  target: 'electron-renderer'
}

const { getPythonVersion } = require('./utils');
const pyVersion = getPythonVersion() || '3'

/**
 * Adjust rendererConfig for development settings
 */
if (process.env.NODE_ENV !== 'production') {
  rendererConfig.plugins.push(
    new webpack.HotModuleReplacementPlugin(),
    new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../resources').replace(/\\/g, '\\\\')}"`,
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.NODE_ENV': '"development"',
      'process.env.APP_TYPE': '"renderer"',
    }),
  )
}

/**
 * Adjust rendererConfig for production settings
 */
if (process.env.NODE_ENV === 'production') {
  rendererConfig.devtool = ''
  rendererConfig.plugins.push(
    new OptimizeJsPlugin({
      sourceMap: false
    }),
    new webpack.DefinePlugin({
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.NODE_ENV': JSON.stringify('production'),
      'process.env.APP_TYPE': '"renderer"',
    }),
    // new BundleAnalyzerPlugin({
    //   analyzerMode: 'static',
    // }),
  )
}

module.exports = rendererConfig
