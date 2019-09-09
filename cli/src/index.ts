
import { initDB } from '@/base';
//@ts-ignore
import { version } from '../package.json';
import { addAccountStrategy } from './commanders/add';
import { listAccountsStrategys } from '@/commanders/list';
// import { removeAccountStrategy } from './commanders/remove';
// import { updateAccountStrategy } from './commanders/update';
// import { switchMdSource } from './commanders/switchMdSsource';
// import { monitPrompt } from './components/index';

const program = require('commander');


program
    .version(version)
    // .option('-r --remove', 'remove a account|strategy ')
    // .option('-u --update', 'update accounts|strategys config')
    // .option('-s --switch-md-source', 'switch md source')
    // .option('-m --monit [type]', 'monit processes of account|strategy|all', 'all')
    .action(async (options: any) => {

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
    .command('list [accounts|strategys]')
    .description('list accounts or strategys or all if no type supplied')
    .action((type: string) => {
        return listAccountsStrategys(type)
        .catch((err: Error): void => console.error(err))
        .finally(() => {
            process.exit(0)
        });
    })

//add
program
    .command('add [account|strategy]')
    .description('add a account or strategy')
    .action((type: string) => {
        return addAccountStrategy(type);
    })

//update
program
    .command('update [account|strategy]')
    .description('update a account or strategy')
    .action((type: string) => {
        // return addAccountStrategy();
    })

//remove
program
    .command('remove')
    .description('remove a account or strategy')
    .action((type: string) => {
        // return addAccountStrategy();
    })

program.parse(process.argv)


initDB()
