const fse = require('fs-extra');
const path = require('path');
const { prebuilt, shell } = require('@kungfu-trader/kungfu-core');
const { customResolve, getKfcPath } = require('../utils');

exports.configure = (writePackageJson = false, writeWorkflows = true) => {
  const cwd = process.cwd().toString();
  const packageJsonPath = path.join(cwd.toString(), 'package.json');
  const packageJson = require(packageJsonPath);
  if (writePackageJson) {
    console.log('> write package.json');
    fse.writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2));
  }
  if (writeWorkflows) {
    console.log('> write workflows');
    const findWorkspaceRoot = require('find-yarn-workspace-root');
    const projectDir = findWorkspaceRoot() || cwd;
    const sdkDir = path.dirname(
      path.dirname(customResolve('@kungfu-trader/kungfu-sdk')),
    );
    const targetDir = path.join(projectDir, '.github', 'workflows');
    const dotGithubCODEOWNERS = path.join(projectDir, '.github', 'CODEOWNERS');
    const templates_bmajor = path.join(
      sdkDir,
      'templates',
      'workflows',
      'bump-major-version.yml',
    );
    const templates_bminor = path.join(
      sdkDir,
      'templates',
      'workflows',
      'bump-minor-version.yml',
    );
    const templates_release_new = path.join(
      sdkDir,
      'templates',
      'workflows',
      'release-new-version.yml',
    );
    const templates_release_verify = path.join(
      sdkDir,
      'templates',
      'workflows',
      'release-verify.yml',
    );
    const templates_own = path.join(
      sdkDir,
      'templates',
      'protection',
      'CODEOWNERS',
    );
    try {
      fse.mkdirSync(path.dirname(targetDir), { recursive: true });
    } catch (e) {
      console.warn(`create ${targetDir} fail`);
    }
    try {
      fse.copySync(
        templates_bmajor,
        path.join(projectDir, '.github', 'workflows', 'bump-major-version.yml'),
        { overwrite: false, errorOnExist: true },
      );
    } catch (e) {
      console.warn(`copy bump-major-version.yml fail`);
    }
    try {
      fse.copySync(
        templates_bminor,
        path.join(projectDir, '.github', 'workflows', 'bump-minor-version.yml'),
        { overwrite: false, errorOnExist: true },
      );
    } catch (e) {
      console.warn(`copy bump-minor-version.yml fail`);
    }
    try {
      fse.copySync(
        templates_release_new,
        path.join(
          projectDir,
          '.github',
          'workflows',
          'release-new-version.yml',
        ),
        { overwrite: false, errorOnExist: true },
      );
    } catch (e) {
      console.warn(`copy release-new-version.yml fail`);
    }
    try {
      fse.copySync(
        templates_release_verify,
        path.join(projectDir, '.github', 'workflows', 'release-verify.yml'),
        { overwrite: false, errorOnExist: true },
      );
    } catch (e) {
      console.warn(`copy release-verify.yml fail`);
    }
    try {
      fse.copySync(templates_own, dotGithubCODEOWNERS, { overwrite: true });
    } catch (e) {
      console.warn(`copy CODEOWNERS fail`);
    }
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
