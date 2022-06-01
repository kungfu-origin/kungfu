const { run } = require('./node-lib.js');
const fs = require('fs');
const glob = require('glob');
const path = require('path');

const clean = () => {
  fs.rmSync('dist', { recursive: true, force: true });
  fs.rmSync('build', { recursive: true, force: true });
};

module.exports = require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  const node_pre_gyp = (cmd, check = true) => {
    const buildType = process.env.npm_package_config_build_type;
    const buildTypeOpt = buildType === 'Debug' ? ['--debug'] : [];
    const yarnArgs = ['run', '-s', 'node-pre-gyp', ...buildTypeOpt, ...cmd];
    run('yarn', yarnArgs, check);
  };

  cli
    .command('install', () => {
      const fallbackBuild = process.env.KF_INSTALL_FALLBACK_BUILD === 'true';
      node_pre_gyp(
        fallbackBuild ? ['install', '--fallback-to-build'] : ['install'],
        fallbackBuild,
      );
    })
    .command('build', () => node_pre_gyp(['configure', 'build']))
    .command('clean', () => clean())
    .command('rebuild', () => {
      clean();
      node_pre_gyp(['configure', 'build']);
    })
    .command('package', () => {
      node_pre_gyp(['package']);
      const prebuilt = glob.sync(
        path.join('build', 'stage', '**', '*.tar.gz'),
      )[0];
      const wheel = glob.sync(path.join('build', 'python', 'dist', '*.whl'))[0];
      if (prebuilt && wheel) {
        const packageDir = path.dirname(prebuilt);
        const wheelFileName = path.basename(wheel);
        console.log(`$ cp ${wheel} ${packageDir}`);
        fs.copyFileSync(wheel, path.join(packageDir, wheelFileName));
      }
    });
});
