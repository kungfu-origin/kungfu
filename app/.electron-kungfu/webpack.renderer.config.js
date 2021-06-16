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

const { getPythonVersion, getViewsConfig } = require('./utils');
const viewsConfig = getViewsConfig();


let rendererConfig = {
  devtool: '#cheap-module-eval-source-map',

  entry: viewsConfig.entry,

  output: {
    filename: 'js/[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../dist/app')
  },
  
  externals: [
    ...Object.keys(dependencies || {})
  ],

  module: {
    rules: [
      { 
        test: /\.css$/, 
        use: ExtractTextPlugin.extract({ 
          fallback: 'style-loader', 
          use: [ 'css-loader' ],
          publicPath: '../'
        }) 
      },
      {
        test: /\.scss$/,
        use: ExtractTextPlugin.extract({
          fallback: 'style-loader', 
          use: ['css-loader', 'sass-loader']
        }),
      },
      {
        test: /\.html$/,
        use: 'vue-html-loader'
      },
      {
        test: /\.ts$/,
        use: 'ts-loader',
        include: [
          path.resolve(__dirname, '..', 'resources'),
          path.resolve(__dirname, '..', 'src'),
          path.resolve(__dirname, '..', 'shared'),
          path.resolve(__dirname, '..', '..', 'shared'),
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
          loader: 'file-loader',
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
    ...viewsConfig.plugins,

    new ExtractTextPlugin({
      filename: `css/[name].css`,
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
      'process.env.NODE_ENV': '"production"',
      'process.env.APP_TYPE': '"renderer"',
    }),
    // new BundleAnalyzerPlugin({
    //   analyzerMode: 'static',
    // }),
  )
}

module.exports = rendererConfig
