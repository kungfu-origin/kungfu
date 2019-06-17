import strategy from './components/strategy/index'
import account from './components/account/index'
import monitor from './components/monitor/index'
import { initDB } from '../../app/src/main/base';
import { version } from '../package.json';
import { addStrategyOrAccount } from './assets/prompts';
var program = require('commander');

program
    .version(version)
    .option('-a --add', 'add account or strategy process')
    .option('-u --update', 'update account or strategy config')
    .option('-r --remove', 'remove account or strategy process')
    .option('-l --list', 'list account or strategy')
    .option('-md --md', 'setting account for accepting market data')
    .option('-m --monit', 'monit processes of accounts or strategys', '')
    .action(async options => {
        const add = options.add;
        if(add) addStrategyOrAccount()





        // export const addAccount = (account_id, source_name, receive_md, config) => {


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
        //         console.log(answers)
        //         const type = answers.monit
        //         if(type === 'strategy') strategy();
        //         if(type === 'account') account();
        //         if(type === 'all') monitor();
        //     })
        // }
    })

program.parse(process.argv)




initDB()
