import strategy from './components/strategy/index'
import account from './components/account/index'
import monitor from './components/monitor/index'
import { initDB } from '../../app/src/main/base';
import { version } from '../package.json';
import { addAccountStrategy } from './commanders/add';
import { listAccountsStrategys } from './commanders/list';
import { removeAccountStrategy } from './commanders/remove';
import { updateAccountStrategy } from './commanders/update';

const program = require('commander');
const inquirer = require('inquirer');

program
    .version(version)
    .option('-l --list', 'list all accounts and strategys')
    .option('-a --add', 'add a account / strategy')
    .option('-r --remove', 'remove a account / strategy ')
    .option('-u --update', 'update a accounts / strategys config')
    .option('-m --monit', 'monit processes of accounts or strategys', '')
    .action(async options => {
        const add = options.add;
        if(add) addAccountStrategy();

        const list = options.list;
        if(list) listAccountsStrategys();

        const remove = options.remove;
        if(remove) removeAccountStrategy();

        const update = options.update;
        if(update) updateAccountStrategy();




        // export const addAccount = (account_id, source_name, receive_md, config) => {


        var monit = options.monit
        if(monit) {
            inquirer.prompt([
                {
                    type: 'list',
                    name: 'monit',
                    message: 'Select a type for monitor ',
                    choices: ['all', 'account', 'strategy']
                }
            ]).then(answers => {
                const type = answers.monit
                if(type === 'strategy') strategy();
                if(type === 'account') account();
                if(type === 'all') monitor();
            })
        }
    })

program.parse(process.argv)




initDB()
