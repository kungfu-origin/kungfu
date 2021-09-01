#!/usr/bin/env node

const path = require('path');
const { spawn } = require('child_process');

const webpack = require('webpack');
const WebpackDevServer = require('webpack-dev-server');
const LOCAL_BIN_PATH = path.join(process.cwd(), 'node_modules', '.bin');

function startDevServer() {
  const devServerOptions = {
    hot: true,
    port: 9090,
  };

  const rendererConfig = require('../.webpack/webpack.renderer.config');
  const rendererCompiler = webpack(rendererConfig);
  const devServer = new WebpackDevServer(rendererCompiler, devServerOptions);

  devServer.listen(null, 'localhost', () => {
    const url = 'http://localhost:9090';
    const mainConfig = require('../.webpack/webpack.main.config');
    const mainCompiler = webpack(mainConfig);

    mainCompiler.hooks.entryOption.call(mainCompiler.options.context, mainCompiler.options.entry);

    const definePlugin = new webpack.DefinePlugin({
      __ELECTRON_WEBPACK_DEV_SERVER_URL__: JSON.stringify(url),
    });

    definePlugin.apply(mainCompiler);

    let child;
    const watcher = mainCompiler.watch({}, (err, status) => {
      if (child) {
        child.kill();
      }

      const mainAssets = Object.keys(status.compilation.assets);

      if (mainAssets.length > 1) {
        console.error(
          'Multiple webpack entries found in the electron main process webpack config, please set only one.',
        );
        process.exit(1);
      }

      const mainOutputFile = path.join(mainCompiler.outputPath, mainAssets[0]);
      child = spawn(path.join(LOCAL_BIN_PATH, 'electron'), [mainOutputFile]);
      child.stdout.on('data', (data) => {
        console.log(data.toString());
      });
      child.stderr.on('data', (data) => {
        console.error(data.toString());
      });
      child.on('close', (code) => {
        devServer.close();
        watcher.close();
        process.exit(code);
      });
    });
  });
}

startDevServer();
