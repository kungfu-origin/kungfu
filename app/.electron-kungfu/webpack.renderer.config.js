'use strict'

process.env.BABEL_ENV = 'renderer'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')

// const ExtractTextPlugin = require('extract-text-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');


const { getPythonVersion } = require('./utils');
const pyVersion = getPythonVersion() || '3'

const isProd = process.env.NODE_ENV === 'production';

module.exports = {
  devtool: 'eval',
  entry: {
    index: path.join(__dirname, '../src/renderer/views/index/main.js'),
    code: path.join(__dirname, '../src/renderer/views/code/main.js'),
    makeOrder: path.join(__dirname, '../src/renderer/views/makeOrder/main.js'),
  },

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
        use: [
          MiniCssExtractPlugin.loader,
          'css-loader'
        ]
        // use: ExtractTextPlugin.extract({ 
        //   fallback: 'style-loader', 
        //   use: [ 'css-loader' ] 
        // }) 
      },
      {
        test: /\.scss$/,
        use: [
          MiniCssExtractPlugin.loader,
          'css-loader',
          'sass-loader'
        ]
        
        // ExtractTextPlugin.extract({
        //   fallback: 'style-loader', 
        //   use: ['css-loader', 'sass-loader']
        // }),
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
        }
      }
    ]
  },

  node: {
    __dirname: process.env.NODE_ENV !== 'production',
    __filename: process.env.NODE_ENV !== 'production'
  },

  plugins: [
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

    // new ExtractTextPlugin({
      // filename: `css/[name].css`,
    // }),

    new MiniCssExtractPlugin(),

    new MonacoWebpackPlugin({
      languages: [
        'python', 'cpp', 'shell', 'json', 'yaml'
      ]
    }),

    new webpack.HotModuleReplacementPlugin(),
    new webpack.DefinePlugin({
      'python_version': `"${pyVersion.toString()}"`,
      'process.env.NODE_ENV': isProd ? '"production"' : '"development"',
      'process.env.APP_TYPE': '"renderer"',
    }),

    !isProd ? new webpack.DefinePlugin({
      '__resources': `"${path.join(__dirname, '../resources').replace(/\\/g, '\\\\')}"`,
    }): null,

  ].filter(p => !!p),

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


