'use strict'

const chalk = require('chalk')
const { say } = require('cfonts')
const webpack = require('webpack')
const cliConfig = require('../.build/webpack.cli.config')

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
    const compiler = webpack(cliConfig)
    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err)
        return
      }

      logStats('Main', stats)
      resolve(true)
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
