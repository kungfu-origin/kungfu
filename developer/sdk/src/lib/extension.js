const archiver = require('archiver');
const axios = require('axios');
const ejs = require('ejs');
const fse = require('fs-extra');
const path = require('path');
const os = require('os');
const { spawnSync } = require('child_process');
const { glob } = require('glob');

const LibPathPattern = path.join(__dirname, 'build', 'libs', '*', '*');

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
  return packageJson.name.replace('@kungfu-trader/', '').replace(/-/g, '_');
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
    path.join(__dirname, 'templates', 'kungfu.cmake'),
    {
      includes: glob.sync(path.join(LibPathPattern, 'include')),
      links: glob.sync(path.join(LibPathPattern, 'lib')),
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
    path.join(__dirname, 'templates', 'CMakeLists.txt'),
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

exports.list = async function (libSiteURL, matchName, matchVersion, listVersion = true, listPlatform = false) {
  const getPlatformImplementations = function (targetLibVersion, sourceLibVersion) {
    if ('lib' in sourceLibVersion) {
      for (const osName in sourceLibVersion['lib']) {
        targetLibVersion[osName] = {};
      }
    }
  };
  const getLibVersions = function (targetLib, sourceLib) {
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

exports.install = async function (libSiteURL, libName, libVersion, platform = detectPlatform(), arch = os.arch()) {
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

  const downloadFiles = async function (localDir, files, buildRemoteURL, onFinish) {
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

  const buildDirPath = function (dir) {
    const dirPath = path.join(__dirname, 'build', 'libs', libName, libVersion, dir);
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

exports.installBatch = async function (libSiteURL, platform = detectPlatform(), arch = os.arch()) {
  const libs = getPackageJson().kungfuDependencies;
  for (const libName in libs) {
    await exports.install(libSiteURL, libName, libs[libName], platform, arch);
  }
};

exports.clean = function (keepLibs = true) {
  fse.removeSync(path.join(process.cwd(), 'build'));
  if (!keepLibs) {
    console.log(`Clean downloaded libs from ${path.join(__dirname, 'build', 'libs')}`);
    fse.removeSync(path.join(__dirname, 'build', 'libs'));
  }
};

exports.configure = function () {
  const packageJson = getPackageJson();
  if (hasSourceFor(packageJson, 'python')) {
    spawnExec('yarn', ['kfc', 'engage', 'pdm', 'makeup']);
    spawnExec('yarn', ['kfc', 'engage', 'pdm', 'install']);
  }
  if (hasSourceFor(packageJson, 'cpp')) {
    generateCMakeFiles(getProjectName(packageJson), packageJson.kungfuBuild);
  }
};

exports.build = function () {
  const packageJson = getPackageJson();
  if (hasSourceFor(packageJson, 'python')) {
    spawnExec('yarn', ['kfc', 'engage', 'nuitka', '--output-dir=build', 'src/python']);
  }
  if (hasSourceFor(packageJson, 'cpp')) {
    spawnExec('yarn', ['cmake-js', 'build']);
  }
};

exports.dist = function () {
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
    ...findFiles(path.join(LibPathPattern, 'lib', '**')),
    path.join(process.cwd(), 'package.json'),
  ].forEach((f) => {
    archive.file(f, { name: path.basename(f) });
  });

  archive.pipe(fse.createWriteStream(distFile));
  archive.on('error', logError);
  archive.finalize();
};
