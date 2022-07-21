const findWorkspaceRoot = require('find-yarn-workspace-root');
const fse = require('fs-extra');
const path = require('path');
const { getPackageJson } = require('./extension');

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
  const packageJson = getPackageJson();
  const extensionName = packageJson.kungfuConfig.key;
  const outputDir = path.join('dist', extensionName);

  fse.copySync(
    require.resolve('@kungfu-trader/kungfu-core/dist/kfc/drone.node'),
    path.join(outputDir, `${packageJson.binary.module_name}.node`),
    {},
  );
  require('@kungfu-trader/kungfu-core').prebuilt('package');
};
