'use strict'

const chalk = require('chalk')
const path = require('path')
const { say } = require('cfonts')
const webpack = require('webpack')
const webpackHotMiddleware = require('webpack-hot-middleware')

const cliConfig = require('./webpack.cli.config')

let electronProcess = null
let hotMiddleware

function logStats (proc, data) {
  let log = ''

  log += chalk.yellow.bold(`┏ ${proc} Process ${new Array((19 - proc.length) + 1).join('-')}`)
  log += '\n\n'

  if (typeof data === 'object') {
    data.toString({
      colors: true,
      chunks: false
    }).split(/\r?\n/).forEach(line => {
      log += '  ' + line + '\n'
    })
  } else {
    log += `  ${data}\n`
  }

  log += '\n' + chalk.yellow.bold(`┗ ${new Array(28 + 1).join('-')}`) + '\n'

  console.log(log)
}


function startCli () {
  return new Promise((resolve, reject) => {
    cliConfig.entry.account = [].concat(cliConfig.entry.account)

    const compiler = webpack(cliConfig)
    hotMiddleware = webpackHotMiddleware(compiler, { 
        log: false, 
        heartbeat: 2500 
      })

    compiler.plugin('watch-run', (compilation, done) => {
      logStats('Main', chalk.white.bold('compiling...'))
      hotMiddleware.publish({ action: 'compiling' })
      done()
    })

    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err)
        return
      }

      logStats('Main', stats)

      if (electronProcess && electronProcess.kill) {
        manualRestart = true
        process.kill(electronProcess.pid)
        electronProcess = null
        startElectron()

        setTimeout(() => {
          manualRestart = false
        }, 5000)
      }

      resolve()
    })
  })
}


function greeting () {
  const cols = process.stdout.columns
  let text = ''

  if (cols > 104) text = ''
  else if (cols > 76) text = 'kungfu-|trader'
  else text = false

  if (text) {
    say(text, {
      colors: ['yellow'],
      font: 'simple3d',
      space: false
    })
  } else console.log(chalk.yellow.bold('\n  kungfu-trader'))
  console.log(chalk.blue('  getting ready...') + '\n')
}

function init () {
  greeting()

  Promise.all([startCli()])
    .catch(err => {
      console.error(err)
    })
}

init()
