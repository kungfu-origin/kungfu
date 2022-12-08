const axios = require('axios');
const ejs = require('ejs');
const fse = require('fs-extra');
const path = require('path');
const os = require('os');
const { spawnSync } = require('child_process');
const { glob } = require('glob');
const { promisify } = require('util');
const { finished } = require('stream');
const {
  customResolve,
  getKfcPath,
  getKfcCmdArgs,
  getCmakeCmdArgs,
  getCmakeNextCmdArgs,
  kfcName,
  dealPath,
} = require('../utils');
const { shell, prebuilt } = require('@kungfu-trader/kungfu-core');
const versioning = require('@mapbox/node-pre-gyp/lib/util/versioning');
const project = require('./project');

const pypackages = '__pypackages__';
const kungfulibs = '__kungfulibs__';
const kungfuLibDirPattern = `${kungfulibs}/*/*`;
const cwd = process.cwd().toString();

const spawnOptsShell = {
  shell: true,
  windowsHide: true,
  cwd: fse.realpathSync(path.join(cwd)),
};

const spawnOptsInherit = {
  ...spawnOptsShell,
  stdio: 'inherit',
};

function spawnExec(cmd, args, opts = spawnOptsInherit) {
  const result = spawnSync(cmd, args, opts);
  if (result.status !== 0) {
    console.error(`Failed to execute $ ${cmd} ${args.join(' ')}`);
    process.exit(result.status);
  }
  return result;
}

function logError(err) {
  return err ? console.error(err) : null;
}

function detectPlatform() {
  const osName = os.platform();
  if (osName === 'darwin') {
    return 'macos';
  }
  if (osName === 'win32') {
    return 'windows';
  }
  return osName;
}
exports.detectPlatform = detectPlatform;

function hasSourceFor(packageJson, language) {
  const config = packageJson.kungfuBuild;
  const hasConfig = config && config[language];
  const source = glob.sync(`src/${language}/**/*.*`);
  const hasSource = source.length > 0;
  return hasConfig || hasSource;
}

function getProjectName(packageJson) {
  return packageJson.kungfuConfig.key;
}

function generateCMakeFiles(projectName, kungfuBuild) {
  const extraSources = {
    'bind/node': '${CMAKE_JS_SRC}',
  };
  const targetMakers = {
    exe: 'add_executable',
    'bind/python': 'pybind11_add_module',
    'bind/node': 'add_library',
  };

  kungfuBuild = kungfuBuild || { cpp: { target: 'bind/python' } };

  const cppSources = [kungfuBuild.cpp.src || ['src/cpp']].flat();

  const cppLinksOpt = kungfuBuild.cpp.links || [];
  const cppLinks = Array.isArray(cppLinksOpt)
    ? cppLinksOpt
    : cppLinksOpt[detectPlatform()];

  const cwd = process.cwd().toString();
  const buildDir = path.join(cwd, 'build');
  fse.ensureDirSync(buildDir);
  const kfcDir = getKfcPath();

  ejs.renderFile(
    customResolve('@kungfu-trader/kungfu-sdk/templates/cmake/kungfu.cmake'),
    {
      kfcDir: dealPath(kfcDir),
      kfcExec: dealPath(path.join(kfcDir, kfcName)),
      includes: glob.sync(`${kungfuLibDirPattern}/include`),
      links: glob.sync(`${kungfuLibDirPattern}/lib`),
      sources: cppSources,
      extraSource: extraSources[kungfuBuild.cpp.target],
      makeTarget: targetMakers[kungfuBuild.cpp.target],
      targetLinks: (cppLinks || ['']).join(' '),
    },
    (err, str) => {
      logError(err) ||
        fse.writeFileSync(path.join(buildDir, 'kungfu.cmake'), str);
    },
  );

  if (kungfuBuild.cpp.cmakeOverride) {
    return;
  }

  ejs.renderFile(
    customResolve('@kungfu-trader/kungfu-sdk/templates/cmake/CMakeLists.txt'),
    {
      projectName: projectName,
    },
    (err, str) => {
      if (err) {
        logError(err);
      }
      fse.writeFileSync(path.join(cwd, 'CMakeLists.txt'), str);
    },
  );
}

const DefaultLibSiteURL_CN = 'https://external.libkungfu.cc';
const DefaultLibSiteURL_US = 'https://external.libkungfu.io';

exports.DefaultLibSiteURL = process.env.GITHUB_ACTIONS
  ? DefaultLibSiteURL_US
  : DefaultLibSiteURL_CN;

exports.list = async (
  libSiteURL,
  matchName,
  matchVersion,
  listVersion = true,
  listPlatform = false,
) => {
  const getPlatformImplementations = (targetLibVersion, sourceLibVersion) => {
    if ('lib' in sourceLibVersion) {
      for (const osName in sourceLibVersion['lib']) {
        targetLibVersion[osName] = {};
      }
    }
  };
  const getLibVersions = (targetLib, sourceLib) => {
    for (const libVersion in sourceLib) {
      if (!libVersion.match(matchVersion)) continue;
      targetLib[libVersion] = {};
      if (listPlatform) {
        getPlatformImplementations(
          targetLib[libVersion],
          sourceLib[libVersion],
        );
      }
    }
  };
  const response = await axios.get(`${libSiteURL}/index.json`);
  const sourceLibs = response.data;
  const targetLibs = {};
  for (const libName in sourceLibs) {
    if (!libName.match(matchName)) continue;
    targetLibs[libName] = {};
    if (listVersion) {
      getLibVersions(targetLibs[libName], sourceLibs[libName]);
    }
  }
  return targetLibs;
};

exports.installSingleLib = async (
  libSiteURL,
  libName,
  libVersion,
  platform = detectPlatform(),
  arch = os.arch(),
) => {
  const localLibDir = path.join(kungfulibs, libName, libVersion);
  if (fse.existsSync(localLibDir)) {
    console.log(
      `-- Lib ${libName}@${libVersion} exists at ${localLibDir}, skip downloading`,
    );
    return;
  }

  const index = await axios.get(`${libSiteURL}/index.json`);
  const sourceLibs = index.data;

  const findVersion = !libVersion || libVersion === '*';

  if (libName in sourceLibs && findVersion) {
    libVersion = Object.keys(sourceLibs[libName]).sort().pop();
  }

  if (!(libName in sourceLibs && libVersion in sourceLibs[libName])) {
    console.error(`-- Failed to find ${libName}@${libVersion}`);
    return;
  }

  const libInfo = sourceLibs[libName][libVersion];

  if (!(platform in libInfo.lib)) {
    console.warn(`-- ${libName}@${libVersion} does not support ${platform}`);
    return;
  }

  const downloadFiles = async (localDir, files, buildRemoteURL, onFinish) => {
    for (const file of files) {
      const remoteFileURL = buildRemoteURL(file);
      const localFilePath = path.join(localDir, file);
      console.log(`-- Downloading ${remoteFileURL} to ${localFilePath}`);

      fse.ensureDirSync(path.dirname(localFilePath));

      const writer = fse.createWriteStream(localFilePath);
      writer.on('finish', () => onFinish(localFilePath));
      writer.on('error', () =>
        console.error(`-- Failed to download ${remoteFileURL}`),
      );

      const response = await axios({
        url: remoteFileURL,
        method: 'GET',
        responseType: 'stream',
      });
      response.data.pipe(writer);

      await promisify(finished)(writer);
    }
  };

  const buildDirPath = (dir) => {
    const dirPath = path.join(localLibDir, dir);
    fse.ensureDirSync(dirPath);
    return dirPath;
  };
  const docDir = buildDirPath('doc');
  const includeDir = buildDirPath('include');
  const binDir = buildDirPath('lib');

  const encode = (file) => encodeURIComponent(file).replace(/%20/g, '+');
  await downloadFiles(
    docDir,
    libInfo.doc,
    (file) => `${libSiteURL}/${libName}/${libVersion}/doc/${encode(file)}`,
    (localFilePath) => fse.chmodSync(localFilePath, '0644'),
  );
  await downloadFiles(
    includeDir,
    libInfo.include,
    (file) => `${libSiteURL}/${libName}/${libVersion}/include/${file}`,
    (localFilePath) => fse.chmodSync(localFilePath, '0644'),
  );
  await downloadFiles(
    binDir,
    libInfo.lib[platform][arch],
    (file) =>
      `${libSiteURL}/${libName}/${libVersion}/lib/${platform}/${arch}/${file.replace(
        /\+/g,
        '%2B',
      )}`,
    (localFilePath) => fse.chmodSync(localFilePath, '0755'),
  );
};

exports.installBatch = async (
  libSiteURL,
  platform = detectPlatform(),
  arch = os.arch(),
) => {
  const packageJson = shell.getPackageJson();
  if ('kungfuDependencies' in packageJson) {
    const libs = packageJson.kungfuDependencies;
    for (const libName in libs) {
      await this.installSingleLib(
        libSiteURL,
        libName,
        libs[libName],
        platform,
        arch,
      );
    }
  }
  if (hasSourceFor(packageJson, 'python')) {
    const { cmd, args0 } = getKfcCmdArgs();
    spawnExec(cmd, [...args0, 'engage', 'pdm', 'makeup']);
    spawnExec(cmd, [...args0, 'engage', 'pdm', 'install']);
  }
};

exports.clean = (keepLibs = true) => {
  fse.removeSync(path.join(process.cwd().toString(), 'build'));
  fse.removeSync(path.join(process.cwd().toString(), 'dist'));
  if (!keepLibs) {
    const rm = (p) => fse.existsSync(p) && fse.removeSync(p);
    rm(pypackages);
    rm(kungfulibs);
  }
};

exports.configure = () => {
  const packageJson = shell.getPackageJson();
  if (hasSourceFor(packageJson, 'cpp')) {
    generateCMakeFiles(getProjectName(packageJson), packageJson.kungfuBuild);
  }
};

exports.compile = () => {
  const packageJson = shell.getPackageJson();
  const extensionName = packageJson.kungfuConfig.key;
  const outputDir = path.join('dist', extensionName);
  const buildTargetDir = path.join('build', 'target');

  fse.ensureDirSync(outputDir);

  if (hasSourceFor(packageJson, 'python')) {
    const srcDir = path.join('src', 'python', extensionName);
    const { cmd, args0 } = getKfcCmdArgs();

    const kfcArgs = [...args0, 'engage', 'nuitka'];
    const nuitkaArgs = [
      '--module',
      '--assume-yes-for-downloads',
      '--remove-output',
      '--no-pyi-file',
      `--include-package=${extensionName}`,
      `--output-dir=${outputDir}`,
      srcDir,
    ];
    spawnExec(cmd, [...kfcArgs, ...nuitkaArgs]);
  }

  if (hasSourceFor(packageJson, 'cpp')) {
    const cmakeCmdArgs = getCmakeCmdArgs();
    console.log('cmake fist step: ', JSON.stringify(cmakeCmdArgs));
    if (cmakeCmdArgs) {
      const { cmd, args } = cmakeCmdArgs;
      spawnExec(cmd, [...args]);
    }

    const nextCmdArgs = getCmakeNextCmdArgs();
    console.log('cmake second step: ', JSON.stringify(nextCmdArgs));
    if (nextCmdArgs) {
      const { cmd, args } = nextCmdArgs;
      spawnExec(cmd, [...args]);
    }
  }

  const cwd = process.cwd().toString(); // 这一步避免在打包中process.cwd()被替换
  const packageJsonPath = path.join(cwd, 'package.json');
  const readmePath = path.join(cwd, 'README.md');
  fse.copyFile(packageJsonPath, path.join(outputDir, 'package.json'));
  if (fse.existsSync(readmePath)) {
    fse.copyFile(readmePath, path.join(outputDir, 'README.md'));
  }

  const copyOutput = (pattern) => {
    glob.sync(pattern).forEach((p) => {
      fse.copyFile(p, path.join(outputDir, path.basename(p)));
    });
  };

  if (fse.existsSync(buildTargetDir)) {
    copyOutput(path.resolve(buildTargetDir, '*'));
  }

  if (fse.existsSync(kungfulibs)) {
    copyOutput(path.resolve(kungfuLibDirPattern, 'lib', '*'));
  }

  if (fse.existsSync(pypackages)) {
    fse.copySync(pypackages, path.join(outputDir, pypackages));
  }
};

exports.format = () => {
  const packageJson = shell.getPackageJson();
  const srcArgv = ['src'];
  if (hasSourceFor(packageJson, 'cpp')) {
    require('@kungfu-trader/kungfu-core/.gyp/run-format-cpp').main(srcArgv);
  }
  if (hasSourceFor(packageJson, 'python')) {
    require('@kungfu-trader/kungfu-core/.gyp/run-format-python').main(srcArgv);
  }
  if (packageJson.kungfuConfig && packageJson.kungfuConfig.ui_config) {
    require('@kungfu-trader/kungfu-core/.gyp/run-format-js').main(srcArgv);
  }
};

function updatePackageJson(packageJson) {
  const config = packageJson.kungfuConfig || { key: 'KungfuTraderStrategy' };
  packageJson.binary = {
    module_name: config.key,
    module_path: `dist/${config.key}`,
    host: 'localhost',
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
  };

  project.makeBinary(updatePackageJson(packageJson));
  prebuilt('package');
};
