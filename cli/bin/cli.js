#!/usr/bin/env node

var version = require('../package.json').version;
var program = require('commander');
var { strategy, account, monitor } = require('../dist/index').default
program
    .version(version)
    .option('-s --strategy', 'show strategy processes')
    .option('-a --account', 'show account processes')
    .option('-m --monit', 'monit processes')
    .action(function(options){
        if(options.strategy) strategy();
        if(options.account) account();
        if(options.monit) monitor();
    })

program.parse(process.argv)