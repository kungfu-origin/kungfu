const archiver = require('archiver');
const axios = require('axios');
const ejs = require('ejs');
const fse = require('fs-extra');
const path = require('path');
const os = require('os');
const { spawnSync } = require('child_process');
const { glob } = require('glob');

const pypackages = '__pypackages__';
const kungfulibs = '__kungfulibs__'
const kungfuLibDirPattern = path.join(kungfulibs, '*', '*');

const spawnOptsShell = {
  shell: true,
  windowsHide: true,
  cwd: fse.realpathSync(path.resolve(process.cwd())),
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

function getPackageJson() {
  const packageJsonPath = path.join(process.cwd(), 'package.json');
  if (!fse.existsSync(packageJsonPath)) {
    console.log('package.json not found');
    return;
  }
  return JSON.parse(fse.readFileSync(packageJsonPath));
}

function hasSourceFor(packageJson, language) {
  return packageJson.kungfuBuild && packageJson.kungfuBuild[language];
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

  const cppSources = [kungfuBuild.cpp.src || ['src/cpp']].flat();

  const cppLinksOpt = kungfuBuild.cpp.links || [];
  const cppLinks = Array.isArray(cppLinksOpt) ? cppLinksOpt : cppLinksOpt[detectPlatform()];

  const buildDir = path.join(process.cwd(), 'build');
  fse.ensureDirSync(buildDir);

  ejs.renderFile(
    require.resolve('@kungfu-trader/kungfu-sdk/templates/kungfu.cmake'),
    {
      kfcDir: path.dirname(require.resolve("@kungfu-trader/kungfu-core/dist/kfc/kungfubuildinfo.json")),
      includes: glob.sync(path.join(kungfuLibDirPattern, 'include')),
      links: glob.sync(path.join(kungfuLibDirPattern, 'lib')),
      sources: cppSources,
      extraSource: extraSources[kungfuBuild.cpp.target],
      makeTarget: targetMakers[kungfuBuild.cpp.target],
      targetLinks: cppLinks.join(' '),
    },
    (err, str) => {
      logError(err) || fse.writeFileSync(path.join(buildDir, 'kungfu.cmake'), str);
    },
  );

  if (kungfuBuild.cpp.cmakeOverride) {
    return;
  }

  ejs.renderFile(
    require.resolve('@kungfu-trader/kungfu-sdk/templates/CMakeLists.txt'),
    {
      projectName: projectName,
    },
    (err, str) => {
      logError(err) || fse.writeFileSync(path.join(process.cwd(), 'CMakeLists.txt'), str);
    },
  );
}

const DefaultLibSiteURL_CN = 'https://external.libkungfu.cc';
const DefaultLibSiteURL_US = 'https://external.libkungfu.io';

exports.DefaultLibSiteURL = process.env.GITHUB_ACTIONS ? DefaultLibSiteURL_US : DefaultLibSiteURL_CN;

exports.detectPlatform = detectPlatform;

exports.list = async (libSiteURL, matchName, matchVersion, listVersion = true, listPlatform = false) => {
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
        getPlatformImplementations(targetLib[libVersion], sourceLib[libVersion]);
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

exports.install = async (libSiteURL, libName, libVersion, platform = detectPlatform(), arch = os.arch()) => {
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

  const downloadFiles = async (localDir, files, buildRemoteURL, onFinish) => {
    for (const file of files) {
      const remoteFileURL = buildRemoteURL(file);
      const localFilePath = path.join(localDir, file);
      console.log(`-- Downloading ${remoteFileURL} to ${localFilePath}`);

      fse.ensureDirSync(path.dirname(localFilePath));

      const writer = fse.createWriteStream(localFilePath);
      const response = await axios({
        url: remoteFileURL,
        method: 'GET',
        responseType: 'stream',
      });

      writer.on('finish', () => onFinish(localFilePath));
      writer.on('error', () => console.error(`-- Failed to download ${remoteFileURL}`));

      response.data.pipe(writer);
    }
  };

  const buildDirPath = (dir) => {
    const dirPath = path.join(kungfulibs, libName, libVersion, dir);
    fse.ensureDirSync(dirPath);
    return dirPath;
  };
  const docDir = buildDirPath('doc');
  const includeDir = buildDirPath('include');
  const binDir = buildDirPath('lib');

  await downloadFiles(
    docDir,
    libInfo.doc,
    (file) => `${libSiteURL}/${libName}/${libVersion}/doc/${file}`,
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
    (file) => `${libSiteURL}/${libName}/${libVersion}/lib/${platform}/${arch}/${file}`,
    (localFilePath) => fse.chmodSync(localFilePath, '0755'),
  );
};

exports.installBatch = async (libSiteURL, platform = detectPlatform(), arch = os.arch()) => {
  const packageJson = getPackageJson();
  if ('kungfuDependencies' in packageJson) {
    const libs = packageJson.kungfuDependencies;
    for (const libName in libs) {
      await exports.install(libSiteURL, libName, libs[libName], platform, arch);
    }
  }
  if (hasSourceFor(packageJson, 'python')) {
    spawnExec('yarn', ['kfc', 'engage', 'pdm', 'makeup']);
    spawnExec('yarn', ['kfc', 'engage', 'pdm', 'install']);
  }
};

exports.clean = (keepLibs = true) => {
  fse.removeSync(path.join(process.cwd(), 'build'));
  if (!keepLibs) {
    const rm = (p) => fse.existsSync(p) && fse.removeSync(p);
    rm(pypackages);
    rm(kungfulibs);
  }
};

exports.configure = () => {
  const packageJson = getPackageJson();
  if (hasSourceFor(packageJson, 'cpp')) {
    generateCMakeFiles(getProjectName(packageJson), packageJson.kungfuBuild);
  }
};

exports.build = () => {
  const packageJson = getPackageJson();
  const extensionName = packageJson.kungfuConfig.key;
  const buildTargetDir = path.join('build', 'target');
  const outputDir = path.join('dist', extensionName);

  if (hasSourceFor(packageJson, 'python')) {
    const srcDir = path.join('src', 'python', extensionName);
    spawnExec('yarn', [
      'kfc',
      'engage',
      'nuitka',
      '--module',
      '--assume-yes-for-downloads',
      '--remove-output',
      '--no-pyi-file',
      `--include-package=${extensionName}`,
      `--output-dir=${outputDir}`,
      srcDir,
    ]);
  }

  if (hasSourceFor(packageJson, 'cpp')) {
    spawnExec('yarn', [
      'cmake-js',
      'build',
    ]);
  }

  const packageJsonPath = path.join(process.cwd(), 'package.json');
  fse.copyFile(packageJsonPath, path.join(outputDir, 'package.json'));

  const copyOutput = (pattern) => {
    glob.sync(pattern).forEach(p => {
      fse.copyFile(p, path.join(outputDir, path.basename(p)));
    });
  }

  if (fse.existsSync(buildTargetDir)) {
    copyOutput(path.join(buildTargetDir, '*'));
  }

  if (fse.existsSync(kungfulibs)) {
    copyOutput(path.join(kungfuLibDirPattern, 'lib', '*'));
  }

  if (fse.existsSync(pypackages)) {
    fse.copySync(pypackages, path.join(outputDir, pypackages));
  }
};

exports.dist = () => {
  const packageJson = getPackageJson();
  const projectName = getProjectName(packageJson);
  const archive = archiver('tar', { gzip: true });
  const buildDir = path.join(process.cwd(), 'build');
  const targetDir = path.join(buildDir, 'target');
  const distDir = path.join(buildDir, 'dist');
  const distFile = path.join(distDir, `${projectName}.tgz`);

  fse.removeSync(distDir);
  fse.ensureDirSync(distDir);

  const isFile = (p) => fse.lstatSync(p).isFile();
  const findFiles = (p) => glob.sync(p).filter(isFile);

  [
    ...findFiles(path.join(targetDir, '*')),
    ...findFiles(path.join(kungfuLibDirPattern, 'lib', '**')),
    path.join(process.cwd(), 'package.json'),
  ].forEach((f) => {
    archive.file(f, { name: path.basename(f) });
  });

  archive.pipe(fse.createWriteStream(distFile));
  archive.on('error', logError);
  archive.finalize();
};
