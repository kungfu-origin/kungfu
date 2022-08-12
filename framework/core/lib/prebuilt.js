const main = (...argv) => {
  const node_pre_gyp = require('@mapbox/node-pre-gyp');
  const shell = require('./shell');

  const Runner = function () {
    const prog = new node_pre_gyp.Run({
      argv: [process.execPath, __filename, ...argv],
    });

    const run = (runner) => {
      const command = prog.todo.shift();
      if (!command) {
        return runner.success();
      }
      if (command.name === 'install') {
        shell.setAutoConfig();
        shell.showAutoConfig();
      }
      prog.commands[command.name](command.args, function (err) {
        if (!err) {
          process.nextTick(() => run(runner));
          return;
        }
        if (
          command.name !== 'install' ||
          '--build-from-source' in command.args
        ) {
          runner.failure(err);
        }
        const msg = 'safely ignore missing binaries (expected to build)';
        try {
          require('npmlog').info('install', msg);
        } catch (e) {
          console.log(msg);
        }
      });
    };

    if (prog.todo.length > 0) {
      return run(this);
    }

    console.log(prog.usage());
  };

  const proto = Runner.prototype;
  proto.failure = (err) => {
    console.error(err);
    process.exit(-1);
  };
  proto.success = () => true;
  proto.onFailure = function (cb) {
    this.failure = cb;
    return this;
  };
  proto.onSuccess = function (cb) {
    this.success = cb;
    return this;
  };

  return new Runner();
};

module.exports = main;

if (require.main === module) {
  main();
}
