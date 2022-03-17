import CFonts from 'cfonts';
import colors from 'colors';
import { version } from '../package.json';
import { Command, program } from 'commander';
import { listKfLocations } from './commanders/list';
import { selectMdTdStrategy, addMdTdStrategy } from './commanders/add';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  BASE_DB_DIR,
  KF_HOME,
  LOG_DIR,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { removeFilesInFolder } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { updateMdTdStrategy } from './commanders/update';
import { removeMdTdStrategy } from './commanders/remove';
import { monitPrompt } from './commanders/monit';
import {
  ensureKungfuKey,
  initKfConfig,
} from '@kungfu-trader/kungfu-js-api/config';
import { shutdown } from './commanders/shutdown';
import 'console-success';
import { removeJournal } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { setGlobalSetting, showGlobalSetting } from './commanders/config';
import { exportTradingDataPrompt } from './commanders/export';

const argvs = process.argv.filter((s) => !!s);
if (argvs[argvs.length - 1] === '-h') {
  console.log(colors.green('Welcome to kungfu trader system'));
  CFonts.say('KungFu', {
    font: 'block', // define the font face
    align: 'left', // define text alignment
    colors: ['yellow'], // define all colors
    background: 'transparent', // define the background color, you can also use `backgroundColor` here as key
    letterSpacing: 2, // define letter spacing
    lineHeight: 2, // define the line height
    space: true, // define if the output text should have empty lines on top and on the bottom
    maxLength: '0', // define how many character can be on one line
  });
}

program.version(version);

program
  .command('monit')
  .option('-l --list', 'list detail')
  .description(
    'monitor all process with merged logs OR monitor one trading process (with -l)',
  )
  .action((type: string, commander: Command) => {
    const list = commander['list'] || false;
    monitPrompt(!!list);
  });

program
  .command('list')
  .description('list mds, tds and strategies')
  .action(() => {
    return listKfLocations()
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('add')
  .description('add a md, td or strategy')
  .action(() => {
    return selectMdTdStrategy()
      .then((type: string) => addMdTdStrategy(type as KfCategoryTypes))
      .then(() => listKfLocations())
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('update')
  .description('update a md, td or strategy')
  .action(() => {
    return updateMdTdStrategy()
      .then(() => listKfLocations())
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('remove')
  .description('remove a md, td or strategy')
  .action(() => {
    return removeMdTdStrategy()
      .then(() => listKfLocations())
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('export')
  .description('Export all trading data by date')
  .action(() => {
    return exportTradingDataPrompt()
      .then((output_path: string) => {
        console.success(`Export trading data to ${output_path} success`);
      })
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('shutdown')
  .description('shutdown all kungfu processes')
  .action(() => {
    shutdown();
  });

program
  .command('config')
  .description('set system config of kungfu')
  .action(async () => {
    try {
      await setGlobalSetting();
      await showGlobalSetting();
      await process.exit(0);
    } catch (err) {
      console.error(err);
      process.exit(1);
    }
  });

program
  .command('clearLog')
  .description('clear all logs (should do it often)')
  .action(() => {
    return removeFilesInFolder(LOG_DIR)
      .then(() => console.success('Clear all logs'))
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('clearJournal')
  .description(
    'clear all journal (Be carefull, this action will clear all trading data)',
  )
  .action(() => {
    return removeJournal(KF_HOME)
      .then(() => console.success('Clear all jouranl files'))
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program
  .command('showdir <home|log|base>')
  .description('show the dir path of home, log or base')
  .action((target: string) => {
    switch (target) {
      case 'home':
        console.log(KF_HOME);
        break;
      case 'log':
        console.log(LOG_DIR);
        break;
      case 'base':
        console.log(BASE_DB_DIR);
        break;
    }
    process.exit(0);
  });

program.on('command:*', function () {
  console.error(
    `Invalid command: ${program.args.join(
      ' ',
    )}\nSee --help for a list of available commands.`,
  );
  process.exit(1);
});

program.parse(process.argv);

initKfConfig();
ensureKungfuKey();
