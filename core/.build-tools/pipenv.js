const fse = require("fs-extra");
const path = require("path");
const {spawn} = require("child_process");

const argv = require("yargs/yargs")(process.argv.slice(2))
    .option("python")
    .option("pypi-mirror")
    .help()
    .argv


const python_opt = argv.python && !fse.existsSync(path.join("Pipfile.lock")) ? ["--python", argv.python] : []
const pypi_opt = argv.pypiMirror ? ["--pypi-mirror", argv.pypiMirror] : []
const pipenv_args = [...python_opt, ...pypi_opt, ...argv._]

const child = spawn("pipenv", pipenv_args, {
    shell: true,
    stdio: ["inherit", "inherit", "inherit"],
    windowsHide: true
});

child.on("error", (error) => {
    process.stderr.write(error)
});