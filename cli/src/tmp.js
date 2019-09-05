const { initDB } = require('./base');
const { logger } = require('__gUtils/logUtils');


initDB()


var version = require('../../package.json').version;
var program = require('commander');
var inquirer = require( 'inquirer' );
// var { strategy, account, monitor } = require('../dist/index').default
program
    .version(version)
    .option('-a --add', 'add accounts or strategys')
    .option('-u --update', 'update accounts or strategys')
    .option('-r --remove', 'remove account or strategy')
    .option('-m --monit', 'monit processes of accounts or strategys', '')
    .action(function(options){
		console.log(options)
        // var add = options.add;
        // if(add){
        //     inquirer.prompt([
        //         {
        //             type: 'list',
        //             name: 'add',
        //             message: 'Add a account or strategy',
        //             choices: ['account', 'strategy']
        //         }
        //     ]).then(answers => {
        //         const type = answers.add
        //         if(type === 'account') {
        //             inquirer.prompt([
        //                 {
        //                     type: 'list',
        //                     name: 'source',
        //                     message: 'Select one type of source',
        //                     choices: ['xtp (stock)', 'ctp (future)']
        //                 }
        //             ])
        //         }
        //     })
        // }


        // var monit = options.monit
        // if(monit) {
        //     inquirer.prompt([
        //         {
        //             type: 'list',
        //             name: 'monit',
        //             message: 'Select a type for monitor ',
        //             choices: ['all', 'account', 'strategy']
        //         }
        //     ]).then(answers => {
        //         const type = answers.monit
        //         if(type === 'strategy') strategy();
        //         if(type === 'account') account();
        //         if(type === 'all') monitor();
        //     })
        // }
    })

program.parse(process.argv)