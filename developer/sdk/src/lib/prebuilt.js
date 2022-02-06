const findWorkspaceRoot = require('find-yarn-workspace-root');
const fse = require('fs-extra');
const path = require('path');

exports.install = () => {
  require('@kungfu-trader/kungfu-core').prebuilt('install');
};

exports.configure = (writePackageJson = false, writeWorkflows = true) => {
  const packageJsonPath = path.join(process.cwd(), 'package.json');
  const packageJson = require(packageJsonPath);
  if (writePackageJson) {
    console.log('> write package.json');
    fse.writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2));
  }
  if (writeWorkflows) {
    console.log('> write workflows');
    const projectDir = findWorkspaceRoot() || process.cwd();
    const srcDir = path.dirname(
      require.resolve(
        '@kungfu-trader/kungfu-sdk/templates/workflows/bump-major-version.yml',
      ),
    );
    const targetDir = path.join(projectDir, '.github', 'workflows');
    fse.mkdirSync(path.dirname(targetDir), { recursive: true });
    fse.copySync(srcDir, targetDir, { overwrite: true });
  }
};

exports.package = () => {
  require('@kungfu-trader/kungfu-core').prebuilt('package');
};
