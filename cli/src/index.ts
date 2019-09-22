
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
import { startLedger, startMaster, killExtra, killGodDaemon, killKfc } from '__gUtils/processUtils';
import { removeFilesInFolder } from '__gUtils/fileUtils';
import { logger } from '__gUtils/logUtils';
import { LIVE_TRADING_DB_DIR, LOG_DIR, BASE_DB_DIR, KF_HOME } from '__gConfig/pathConfig';

const CFonts = require('cfonts');
const colors = require('colors');

startMaster(false)
    .catch(() => {})
    .finally(() => {   
        delaySeconds(1000)
        .then(() => startLedger(false))
        .catch(() => {})
    })

if(process.argv.length === 2 || process.argv[2] === '-h') {
    console.log(colors.green('Welcome to kungfu trader system'))
    CFonts.say('KungFu', {
        font: 'block',              // define the font face
        align: 'left',              // define text alignment
        colors: ['yellow'],         // define all colors
        background: 'transparent',  // define the background color, you can also use `backgroundColor` here as key
        letterSpacing: 2,           // define letter spacing
        lineHeight: 2,              // define the line height
        space: true,                // define if the output text should have empty lines on top and on the bottom
        maxLength: '0',             // define how many character can be on one line
    });
    if( process.argv.length === 2 ) process.exit(0)
}

const program = require('commander');

program
    .version(version)
    .option('-l --list', 'list target process to monit');

program
    .command('monit [-l]')
    .description('monitor all process with merged logs OR monitor one trading process')
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
    .command('switchmd')
    .description('switch md source')
    .action(() => {
        return switchMdSource()
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0));
    })

program
    .command('shutdown')
    .description('shutdown all kungfu processes')
    .action(async () => {
        try {
            await killKfc();
            await killGodDaemon();
            await killExtra();
            console.log('Shutdown kungfu successfully!')
        } catch (err) {
            console.log(err)
        }
        
        process.exit(0)
    })

program
    .command('clearlog')
    .description('clear all logs (should do it often)')
    .action(() => {
        return removeFilesInFolder(LOG_DIR)
            .then(() => console.log('Clear all logs successfully!'))
            .catch((err: Error) => console.error(err))
            .finally(() => process.exit(0))
    })

program
    .command('showdir <home|log|ledger|base>')
    .description('show the dir path of home or log or ledger or base')
    .action((target: string) => {
        switch (target) {
            case 'home':
                console.log(KF_HOME)
                break;
            case 'log':
                console.log(LOG_DIR)
                break;
            case 'ledger':
                console.log(LIVE_TRADING_DB_DIR)
                break;
            case 'base':
                console.log(BASE_DB_DIR)
                break;
        }
        process.exit(0)
    })

program
    .on('command:*', function () {
        console.error('Invalid command: %s\nSee --help for a list of available commands.', program.args.join(' '));
        process.exit(1);
        
    });



program.parse(process.argv)



initDB()

