const fs = require('fs');
const glob = require('glob');
const path = require('path');
const { prebuilt, shell } = require('../lib');

function build() {
  shell.showAutoConfig();
  callPrebuilt(['configure', 'build']);
}

function clean() {
  fs.rmSync('dist', { recursive: true, force: true });
  fs.rmSync('build', { recursive: true, force: true });
}

function callPrebuilt(args, check = true) {
  const buildType = process.env.npm_package_config_build_type;
  const buildTypeOpt = buildType === 'Debug' ? ['--debug'] : [];
  return prebuilt(...buildTypeOpt, ...args);
}

module.exports = require('../lib/sywac')(module, (cli) => {
  cli
    .command('install', () => {
      const fallbackBuild = process.env.KF_INSTALL_FALLBACK_BUILD === 'true';
      callPrebuilt(
        fallbackBuild ? ['install', '--fallback-to-build'] : ['install'],
        fallbackBuild,
      );
    })
    .command('build', () => build())
    .command('clean', () => clean())
    .command('rebuild', () => {
      clean();
      build();
    })
    .command('package', () => {
      callPrebuilt(['package']).onSuccess(() => {
        const prebuilt = glob.sync('build/stage/**/*.tar.gz');
        const wheel = glob.sync('build/python/dist/*.whl');
        if (prebuilt.length > 0 && wheel.length > 0) {
          const packageDir = path.dirname(prebuilt[0]);
          const wheelFileName = path.basename(wheel[0]);
          console.log(`$ cp ${wheel} ${packageDir}`);
          fs.copyFileSync(wheel[0], path.join(packageDir, wheelFileName));
        }
      });
    });
});
