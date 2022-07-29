const { shell, prebuilt } = require('@kungfu-trader/kungfu-core');
const versioning = require('@mapbox/node-pre-gyp/lib/util/versioning');
const project = require('./project');

function updatePackageJson(packageJson) {
  const config = packageJson.kungfuConfig || { key: 'KungfuTraderStrategy' };
  packageJson.binary = {
    module_name: config.key,
    module_path: `dist/${config.key}`,
    host: 'localhost'
  };
  packageJson.main = 'package.json';
  return packageJson;
}

exports.package = () => {
  const packageJson = shell.getPackageJson();

  if (packageJson.host) {
    project.package();
    return;
  }

  const evaluate = versioning.evaluate;

  versioning.evaluate = (packageJson, options, napiBuildVersion) => {
    updatePackageJson(packageJson);
    return evaluate(packageJson, options, napiBuildVersion);
  }

  project.makeBinary(updatePackageJson(packageJson));
  prebuilt('package');
}