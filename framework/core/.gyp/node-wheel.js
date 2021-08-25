const fse = require('fs-extra');
const path = require('path');
const { spawnSync } = require('child_process');

const buildType = process.env.npm_package_config_build_type;
const srcDir = path.join('src', 'python');
const buildDir = path.join('build', buildType);
const wheelDir = path.join('build', 'python');

fse.removeSync(wheelDir);
fse.copySync(srcDir, wheelDir, { filter: (p) => !path.basename(p).startsWith('kfc.') });
fse.copySync(buildDir, wheelDir, { filter: (p) => !path.basename(p).endsWith('.node') });

const pipenv_args = ['run', 'python', 'setup.py', 'bdist_wheel'];

console.log(`$ pipenv ${pipenv_args.join(' ')}`);

const result = spawnSync('pipenv', pipenv_args, {
  shell: true,
  stdio: 'inherit',
  windowsHide: true,
  cwd: wheelDir,
});

process.exit(result.status);
