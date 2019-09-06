
import { initDB } from '@/base';
//@ts-ignore
import { version } from '../package.json';
// import { addAccountStrategy } from './commanders/add';
import { listAccountsStrategys } from '@/commanders/list';
// import { removeAccountStrategy } from './commanders/remove';
// import { updateAccountStrategy } from './commanders/update';
// import { switchMdSource } from './commanders/switchMdSsource';
// import { monitPrompt } from './components/index';

const program = require('commander');

program
    .version(version)
    // .option('-a --add', 'add a account|strategy')
    // .option('-r --remove', 'remove a account|strategy ')
    // .option('-u --update', 'update accounts|strategys config')
    // .option('-s --switch-md-source', 'switch md source')
    // .option('-m --monit [type]', 'monit processes of account|strategy|all', 'all')
    .action(async (options: any) => {
        // const add = options.add;
        // if(add) return addAccountStrategy();

        // const remove = options.remove;
        // if(remove) return removeAccountStrategy();
        // const update = options.update;
        // if(update) return updateAccountStrategy();
        // const sms = options.switchMdSource;
        // if(sms) return switchMdSource();
        // var monit = options.monit
        // if(monit) return monitPrompt(monit);
    })


//list
program
    .command('list [types]')
    .description('list "accounts" or "strategys" or all if no type supplied')
    .action((type: string) => {
        listAccountsStrategys(type)
        .catch((err: Error): void => console.error(err))
        .finally(() => {
            process.exit(0)
        });
    })

//add
program
    .command('add')
    .description('add "accounts" or "strategys"')
    .action((type: string) => {

    })

program.parse(process.argv)




initDB()
