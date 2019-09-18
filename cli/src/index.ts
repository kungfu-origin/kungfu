
import { initDB } from '@/base';
//@ts-ignore
import { version } from '../package.json';
import { addAccountStrategy } from '@/commanders/add';
import { listAccountsStrategys } from '@/commanders/list';
import { removeAccountStrategy } from '@/commanders/remove';
import { updateAccountStrategy } from '@/commanders/update';
import { switchMdSource } from '@/commanders/switchMdSsource';
import { monitPrompt } from '@/components/index';
import { delaySeconds } from '__gUtils/busiUtils';
import { startLedger, startMaster } from '__gUtils/processUtils';
import { logger } from '__gUtils/logUtils';

const program = require('commander');

program
    .version(version)
    .option('-l --list', 'list target process to monit');

program
    .command('monit [options]')
    .description('monitor target process')
    .action((type: any, commander: any) => {
        const list = commander.parent.list
        return monitPrompt(!!list)
    })

//list
program
    .command('list')
    .description('list accounts and strategies')
    .action(() => {
        return listAccountsStrategys()
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

//add
program
    .command('add <account|strategy>')
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
    .action(() => {
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

if (process.env.NODE_ENV === 'development') {
    program.parse(process.argv)
} else {
    //@ts-ignore
    program.parse([null].concat(process.argv))
}

initDB()

startMaster(false)
    .catch(() => {})
    .finally(() => {
        delaySeconds(1000)
        .then(() => startLedger(false))
        .catch(() => {})
    })

