module.exports = {
  module: {
    rules: [
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
        test: /log4js\/lib\/appenders\/index\.js$/,
        loader: "string-replace-loader",
        options: {
          search: "return require(modulePath);",
          replace: "return coreAppenders['file'];"
        }
      }
    ]
  }
}