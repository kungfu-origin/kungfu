const { run } = require('./node-lib.js');
const fs = require('fs');
const glob = require('glob');
const path = require('path');

require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  const node_pre_gyp = (cmd, check = true) => {
    const buildType = process.env.npm_package_config_build_type;
    const buildTypeOpt = buildType === 'Debug' ? ['--debug'] : [];
    const yarnArgs = ['node-pre-gyp', ...buildTypeOpt, ...cmd];
    run('yarn', yarnArgs, check);
  };

  cli
    .command('install', () => {
      const skipBuild = process.env.KF_SKIP_FALLBACK_BUILD;
      node_pre_gyp(
        skipBuild ? ['install'] : ['install', '--fallback-to-build'],
        !skipBuild,
      );
    })
    .command('build', () => node_pre_gyp(['configure', 'build']))
    .command('clean', () => node_pre_gyp(['clean']))
    .command('rebuild', () => node_pre_gyp(['rebuild']))
    .command('package', () => {
      node_pre_gyp(['package']);
      const prebuilt = glob.sync(
        path.join('build', 'stage', '**', '*.tar.gz'),
      )[0];
      const wheel = glob.sync(path.join('build', 'python', 'dist', '*.whl'))[0];
      if (prebuilt && wheel) {
        console.log(`$ cp ${wheel} ${path.dirname(prebuilt)}`);
        fs.copyFileSync(
          wheel,
          path.join(path.dirname(prebuilt), path.basename(wheel)),
        );
      }
    });
});
