

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

// Install `electron-debug` with `devtron`
require('electron-debug')()

// Install `vue-devtools`
app.on('ready', async () => {

  try {
    await session.defaultSession.loadExtension(path.resolve('../../\.devtolls/vue-devtool'));
  } catch (err) {
    console.error(err)
  }
})


// Require `main` process to boot app
require('./index')
