const fse = require('fs-extra');
const { glob } = require('glob');
const path = require('path');
const { prebuilt, shell } = require('@kungfu-trader/kungfu-core');
const { customResolve, getKfcPath } = require('../utils');

exports.configure = (writePackageJson = false, writeWorkflows = true) => {
  const cwd = process.cwd().toString();
  const packageJsonPath = path.join(cwd.toString(), 'package.json');
  const packageJson = require(packageJsonPath);
  if (writePackageJson) {
    console.log('-- writing package.json ...');
    fse.writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2));
    console.log(`-- written to ${packageJsonPath}`);
  }
  if (writeWorkflows) {
    console.log('-- writing workflows ...');
    const findWorkspaceRoot = require('find-yarn-workspace-root');
    const projectDir = findWorkspaceRoot() || cwd;
    const sdkDir = path.dirname(
      path.dirname(customResolve('@kungfu-trader/kungfu-sdk')),
    );
    const githubTemplateDir = path.join(sdkDir, 'templates', 'github');
    const githubConfigDir = path.join(projectDir, '.github');
    glob
      .sync(`${githubTemplateDir}/**`, {})
      .filter((p) => path.resolve(p) !== githubTemplateDir)
      .forEach((p) => {
        const config = path.relative(githubTemplateDir, p);
        if (fse.statSync(p).isDirectory()) {
          const targetDir = path.join(githubConfigDir, config);
          if (!fse.pathExistsSync(targetDir)) {
            console.log('-- mkdir', targetDir);
            fse.mkdirSync(targetDir, { recursive: true });
          }
          return;
        }
        const targetFile = path.join(githubConfigDir, config);
        if (fse.pathExistsSync(targetFile)) {
          console.debug(`-- ${targetFile} exists, skip copy`);
          return;
        }
        try {
          fse.copySync(p, targetFile);
          console.log(`-- copy ${config} to ${targetFile}`);
        } catch (e) {
          console.error(`-- copy ${config} to ${targetFile} failed`);
          console.error(e);
        }
      });
    console.log(`-- written to ${githubConfigDir}`);
  }
};

exports.makeBinary = (packageJson = shell.getPackageJson()) => {
  const outputDir = path.join(packageJson.binary.module_path);

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
