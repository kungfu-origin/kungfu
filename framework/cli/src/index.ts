import CFonts from 'cfonts';
import colors from 'colors';
import { version } from '../package.json';
import { program } from 'commander';
import { listKfLocations } from './commanders/list';
import { selectAccountOrStrategy, addMdTdStrategy } from './commanders/add';

if (process.argv.length === 2 || process.argv[2] === '-h') {
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

program
  .version(version)
  .option('-l --list', 'list detail')
  .option('-a --add', 'add [for ext]')
  .option('-r --remove', 'remove [for ext]')
  .option('-k --kill', 'kill [for monit]');

program.on('command:*', function () {
  console.error(
    `Invalid command: ${program.args.join(
      ' ',
    )}\nSee --help for a list of available commands.`,
  );
  process.exit(1);
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

//add
program
  .command('add')
  .description('add a md, td or strategy')
  .action(() => {
    return selectAccountOrStrategy()
      .then((type: string) => addMdTdStrategy(type))
      .catch((err: Error) => {
        console.error(err);
        process.exit(1);
      })
      .finally(() => process.exit(0));
  });

program.parse(process.argv);
