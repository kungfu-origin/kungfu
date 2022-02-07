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
          console.log(err.message);
          return;
        }
        process.nextTick(run);
      });
  };
  run();
};
