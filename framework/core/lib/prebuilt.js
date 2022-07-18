module.exports = (...argv) => {
  const node_pre_gyp = require('@mapbox/node-pre-gyp');
  const log = require('npmlog');
  const prog = new node_pre_gyp.Run({
    argv: [process.execPath, __filename, ...argv],
  });
  const run = () => {
    const command = prog.todo.shift();
    command &&
      prog.commands[command.name](command.args, function (err) {
        if (!err) {
          process.nextTick(run);
          return;
        }
        if (command.name !== 'install') {
          console.error(err);
          process.exit(-1);
        }
        log.info(
          'install',
          'safely ignore missing binaries (expected to build)',
        );
      });
  };
  run();
};
