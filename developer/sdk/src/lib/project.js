const fse = require('fs-extra');
const path = require('path');
const { prebuilt, shell } = require('@kungfu-trader/kungfu-core');
const { customResolve, getKfcPath } = require('../utils');

exports.configure = (writePackageJson = false, writeWorkflows = true) => {
  const packageJsonPath = path.join(process.cwd(), 'package.json');
  const packageJson = require(packageJsonPath);
  if (writePackageJson) {
    console.log('> write package.json');
    fse.writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2));
  }
  if (writeWorkflows) {
    console.log('> write workflows');
    const findWorkspaceRoot = require('find-yarn-workspace-root');
    const projectDir = findWorkspaceRoot() || process.cwd();
    const srcDir = customResolve(
      '@kungfu-trader/kungfu-sdk/templates/workflows',
    );
    const targetDir = path.join(projectDir, '.github', 'workflows');
    fse.mkdirSync(path.dirname(targetDir), { recursive: true });
    fse.copySync(srcDir, targetDir, { overwrite: true });
  }
};

exports.makeBinary = (packageJson = shell.getPackageJson()) => {
  const outputDir = path.resolve(packageJson.binary.module_path);

  fse.copySync(
    path.join(getKfcPath(), 'drone.node'),
    path.join(outputDir, `${packageJson.binary.module_name}.node`),
    {},
  );
};

exports.package = () => {
  exports.makeBinary();
  prebuilt('package');
};
