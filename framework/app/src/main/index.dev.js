/**
 * This file is used specifically and only for development. It installs
 * `electron-debug` & `vue-devtools`. There shouldn't be any need to
 *  modify this file, but it can be used to extend your development
 *  environment.
 */

/* eslint-disable */

// Set environment for development
const { app, BrowserWindow } = require('electron');
const { default: installExtension, VUEJS_DEVTOOLS } = require('electron-devtools-installer');
const os = require('os');

// Install `vue-devtools`
app.whenReady().then(async () => {
  await installExtension(VUEJS_DEVTOOLS, {
    loadExtensionOptions: { allowFileAccess: true },
    forceDownload: false,
  }).catch((err) => {
    console.log('Unable to install `vue-devtools`: \n', err);
    const chromeData = `${os.homedir()}/Library/Application Support/Google/Chrome`;
    const ext = `${chromeData}/Default/Extensions/nhdogjmejiglipccpnnnanhbledajbpd/5.3.4_0`;
    if (fs.existsSync(ext)) {
      global.__static = require('path').join(__dirname, '/static').replace(/\\/g, '\\');
      BrowserWindow.addDevToolsExtension(ext);
    }
  });
});

// Require `main` process to boot app
require('./index');
