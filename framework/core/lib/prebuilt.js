module.exports = (...argv) => {
  const node_pre_gyp = require('@mapbox/node-pre-gyp');
  const prog = new node_pre_gyp.Run({
    argv: [process.execPath, __filename, ...argv],
  });
  const run = () => {
    const command = prog.todo.shift();
    command &&
      prog.commands[command.name](command.args, function (err) {
        if (err) {
          console.error(err);
          process.exit(-1);
        }
        process.nextTick(run);
      });
  };
  run();
};
