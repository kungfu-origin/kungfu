const findWorkspaceRoot = require('find-yarn-workspace-root');
const fse = require('fs-extra');
const path = require('path');
const { glob } = require('glob');
const { getPackageJson } = require('./extension');

const pypackages = '__pypackages__';
const kungfulibs = '__kungfulibs__';
const kungfuLibDirPattern = path.join(kungfulibs, '*', '*');

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
  const buildTargetDir = path.join('build', 'target');
  const outputDir = path.join('dist', extensionName);

  const copyOutput = (pattern) => {
    glob.sync(pattern).forEach((p) => {
      fse.copyFile(p, path.join(outputDir, path.basename(p)));
    });
  };

  if (fse.existsSync(buildTargetDir)) {
    copyOutput(path.join(buildTargetDir, '*'));
  }

  if (fse.existsSync(kungfulibs)) {
    copyOutput(path.join(kungfuLibDirPattern, 'lib', '*'));
  }

  if (fse.existsSync(pypackages)) {
    fse.copySync(pypackages, path.join(outputDir, pypackages));
  }

  fse.copySync(
    require.resolve('@kungfu-trader/kungfu-core/dist/kfc/drone.node'),
    path.join(outputDir, `${packageJson.binary.module_name}.node`),
    {},
  );
  require('@kungfu-trader/kungfu-core').prebuilt('package');
};
