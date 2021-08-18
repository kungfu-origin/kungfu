module.exports = {
  module: {
    rules: [
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