const fs = require("fs");
const fse = require("fs-extra");
const path = require("path");
const {spawnSync} = require("child_process");

const argv = require("yargs/yargs")(process.argv.slice(2))
    .option("python")
    .option("pypi-mirror")
    .help()
    .argv;

const lock_path = path.join("Pipfile.lock");
const python_opt = argv.python && !fse.existsSync(lock_path) ? ["--python", argv.python] : [];
const pypi_opt = argv.pypiMirror ? ["--pypi-mirror", argv.pypiMirror] : [];
const pipenv_args = [...python_opt, ...pypi_opt, ...argv._];

console.log(`$ pipenv ${pipenv_args.join(' ')}`);

const result = spawnSync("pipenv", pipenv_args, {
    shell: true,
    stdio: ["inherit", "inherit", "inherit"],
    windowsHide: true
});

if (result.status === 0) {
    const time = new Date();
    fs.utimesSync(lock_path, time, time);
}

process.exit(result.status);