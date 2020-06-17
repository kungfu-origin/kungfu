import './base';
//@ts-ignore
import { version } from '../package.json';
import { addAccountStrategy, selectAccountOrStrategy } from '@/commanders/add';
import { listAccountsStrategys } from '@/commanders/list';
import { updateAccountStrategy } from '@/commanders/update';
import { removeAccountStrategy } from '@/commanders/remove';
import { addExtension, listExtension, removeExtension } from "@/commanders/ext";
import { setSystemConfig } from '@/commanders/config';
import { shutdown } from '@/commanders/shutdown';
import { monitKill } from '@/commanders/monitKill';

import { monitPrompt } from '@/components/index';
import { kfKill } from '__gUtils/processUtils';
import { removeFilesInFolder } from '__gUtils/fileUtils';
import { LOG_DIR, BASE_DB_DIR, KF_HOME } from '__gConfig/pathConfig';
import { logger } from '__gUtils/logUtils';

const CFonts = require('cfonts');
const colors = require('colors');

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
}

const program = require('commander');

program
    .version(version)
    .option('-l --list', 'list detail')
    .option('-a --add', 'add [for ext]')
    .option('-r --remove', 'remove [for ext]' )
    .option('-k --kill', 'kill [for monit]');

program
    .command('monit [options]')
    .description('monitor all process with merged logs OR monitor one trading process (with -l)')
    .action((type: any, commander: any) => {
        if (commander.parent.kill) {
            return monitKill()
        }
        const list = commander.parent.list;
        return monitPrompt(!!list)
    })

//list
program
    .command('list')
    .description('list mds, tds and strategies')
    .action(() => {
        return listAccountsStrategys()
            .catch((err: Error) => {
                console.error(err)
                process.exit(1)
            })
            .finally(() => process.exit(0));
    })

//add
program
    .command('add')
    .description('add a md, td or strategy')
    .action(() => {
        return selectAccountOrStrategy()
            .then((type: string) => addAccountStrategy(type))
            .catch((err: Error) => {
                console.error(err)
                process.exit(1)
            })
            .finally(() => process.exit(0));
    })

//update
program
    .command('update')
    .description('update a md, td or strategy')
    .action(() => {
        return updateAccountStrategy()
            .catch((err: Error) => {
                console.error(err)
                process.exit(1)
            })
            .finally(() => process.exit(0));
    })

//remove
program
    .command('remove')
    .description('remove a md, td or strategy')
    .action(() => {
        return removeAccountStrategy()
            .catch((err: Error) => {
                console.error(err)
                process.exit(1)
            })
            .finally(() => process.exit(0));
    })

program
    .command('ext [options]')
    .description('list or add or remove extension (with -l|-a|-r)')
    .action(async (type: any, commander: any) => {
        const list = commander.parent.list
        const add = commander.parent.add
        const remove = commander.parent.remove

        if(!list && !add && !remove) {
            console.error("Missing required options argument -l|-a|-r")
            process.exit(1)
        }

        try {
            if(list) await listExtension()
            else if(add) await addExtension()
            else if(remove) await removeExtension()
            process.exit(0)
        } catch (err) {
            console.error(err)
            process.exit(1)
        }
    })

program
    .command('config')
    .description('set system config of kungfu')
    .action(async (type: any, commander: any) => {
        try {
            await setSystemConfig()
            process.exit(0)
        } catch (err) {
            console.error(err)
            process.exit(1)
        }
    })

program
    .command('shutdown')
    .description('shutdown all kungfu processes')
    .action(() => {
        shutdown()
    })

program
    .command('clearlog')
    .description('clear all logs (should do it often)')
    .action(() => {
        return removeFilesInFolder(LOG_DIR)
            .then(() => console.success('Clear all logs'))
            .catch((err: Error) => {
                console.error(err)
                process.exit(1)
            })
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


