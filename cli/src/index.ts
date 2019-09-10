
import { initDB } from '@/base';
//@ts-ignore
import { version } from '../package.json';
import { addAccountStrategy } from '@/commanders/add';
import { listAccountsStrategys } from '@/commanders/list';
import { removeAccountStrategy } from './commanders/remove';
import { updateAccountStrategy } from '@/commanders/update';
import { switchMdSource } from './commanders/switchMdSsource';
// import { monitPrompt } from './components/index';

const program = require('commander');


program
    .version(version)
    // .option('-s --switch-md-source', 'switch md source')
    // .option('-m --monit [type]', 'monit processes of account|strategy|all', 'all')
    .action(async (options: any) => {
        // const sms = options.switchMdSource;
        // if(sms) return switchMdSource();
        // var monit = options.monit
        // if(monit) return monitPrompt(monit);
    })


//list
program
    .command('list')
    .description('list accounts and strategys')
    .action(() => {
        return listAccountsStrategys()
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

//add
program
    .command('add [account|strategy]')
    .description('add a account or strategy')
    .action((type: string) => {
        return addAccountStrategy(type)
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

//update
program
    .command('update')
    .description('update a account or strategy')
    .action(() => {
        return updateAccountStrategy()
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

//remove
program
    .command('remove')
    .description('remove a account or strategy')
    .action((type: string) => {
        return removeAccountStrategy()
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

program
    .command('switchMd')
    .description('switch md source')
    .action(() => {
        return switchMdSource()
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

program.parse(process.argv)


initDB()
