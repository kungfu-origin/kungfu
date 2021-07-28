const { run } = require('./node-lib.js');
const fs = require('fs');
const glob = require('glob');
const path = require('path');

function node_pre_gyp(cmd, check = true) {
  const build_type = process.env.npm_package_config_build_type;
  const build_type_arg = build_type === 'Debug' ? ['--debug'] : [];
  const yarn_cmd_arg = ['node-pre-gyp', ...build_type_arg, ...cmd];
  run('yarn', yarn_cmd_arg, check);
}

exports.argv = require('yargs/yargs')(process.argv.slice(2))
  .command(
    (command = 'install'),
    (description = 'install through node-pre-gyp'),
    (builder = () => {}),
    (handler = () => {
      const skipBuild = 'KF_SKIP_FALLBACK_BUILD' in process.env;
      node_pre_gyp(skipBuild ? ['install'] : ['install', '--fallback-to-build'], !skipBuild);
    }),
  )
  .command(
    (command = 'build'),
    (description = 'build from source'),
    (builder = () => {}),
    (handler = () => {
      node_pre_gyp(['configure', 'build']);
    }),
  )
  .command(
    (command = 'rebuild'),
    (description = 'rebuild from source'),
    (builder = () => {}),
    (handler = () => {
      node_pre_gyp(['rebuild']);
    }),
  )
  .command(
    (command = 'package'),
    (description = 'package binary'),
    (builder = () => {}),
    (handler = () => {
      node_pre_gyp(['package']);
      const prebuilt = glob.sync(path.join('build', 'stage', '**', '*.tar.gz'))[0];
      const wheel = glob.sync(path.join('dist', '*.whl'))[0];
      if (prebuilt && wheel) {
        console.log(`$ cp ${wheel} ${path.dirname(prebuilt)}`);
        fs.copyFileSync(wheel, path.join(path.dirname(prebuilt), path.basename(wheel)));
      }
    }),
  )
  .demandCommand()
  .help().argv;
