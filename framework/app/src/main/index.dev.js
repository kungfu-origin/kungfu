/**
 * This file is used specifically and only for development. It installs
 * `electron-debug` & `vue-devtools`. There shouldn't be any need to
 *  modify this file, but it can be used to extend your development
 *  environment.
 */

/* eslint-disable */

// Set environment for development
const { app, BrowserWindow } = require('electron');
const os = require('os');

// Install `electron-debug` with `devtron`
require('electron-debug')()

// Install `vue-devtools`
app.on('ready', () => {
  let installExtension = require('electron-devtools-installer')
  installExtension.default(installExtension.VUEJS_DEVTOOLS)
    .then(() => {})
    .catch(err => {
      console.log('Unable to install `vue-devtools`: \n', err)
      global.__static = require('path').join(__dirname, '/static').replace(/\\/g, '\\');
      const chromeData = `${os.homedir()}/Library/Application Support/Google/Chrome`;
      const ext = `${chromeData}/Default/Extensions/nhdogjmejiglipccpnnnanhbledajbpd/5.3.4_0`;
      if (fs.existsSync(ext)) {
        BrowserWindow.addDevToolsExtension(ext);
      }
    })
})

// Require `main` process to boot app
require('./index')
