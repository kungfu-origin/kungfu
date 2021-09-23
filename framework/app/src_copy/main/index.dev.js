/**
 * This file is used specifically and only for development. It installs
 * `electron-debug` & `vue-devtools`. There shouldn't be any need to
 *  modify this file, but it can be used to extend your development
 *  environment.
 */

/* eslint-disable */

// Set environment for development
const { app, session } = require('electron');
const path = require("path");

require('electron-debug')()

// Install `vue-devtools`
app.on('ready', () => {
  const extPath = path.resolve(__dirname, '..', '..', '.devtools', 'vue-devtools');
  session.defaultSession.loadExtension(extPath);
})

// Require `main` process to boot app
require('./index');