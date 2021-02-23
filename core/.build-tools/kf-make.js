#!/usr/bin/env node

const {spawnSync} = require("child_process");
const fs = require("fs");
const path = require("path");

const packageBaseDir = path.resolve(path.dirname(__dirname));

const spawnOptsShell = {
    shell: true,
    windowsHide: true,
    cwd: fs.realpathSync(path.resolve(process.cwd()))
};

const spawnOptsInherit = {
    ...spawnOptsShell,
    stdio: "inherit"
};

const spawnOptsPipe = {
    ...spawnOptsShell,
    stdio: "pipe"
};

function spawnExec(cmd, args, opts) {
    const result = spawnSync(cmd, args, opts);
    if (result.status !== 0) {
        console.error(`Failed to execute $ ${cmd} ${args.join(' ')}`);
        process.exit(result.status);
    }
    return result;
}

const venvPath = (() => {
    const result = spawnExec("pipenv", ["--venv"], {...spawnOptsPipe, cwd: packageBaseDir});
    return path.resolve(result.output.filter(e => e && e.length > 0).pop().slice(0, -1).toString());
})();

function pipenvRun(cmd, args = []) {
    return spawnExec(path.join(venvPath, "bin", cmd), args, spawnOptsInherit);
}

require("yargs/yargs")(process.argv.slice(2))
    .command("configure", "Generates related files", (yargs) => {
    }, (argv) => {
        const opts = {...spawnOptsInherit, cwd: packageBaseDir};
        spawnExec("yarn", ["pipenv-js", "install"], opts);
        spawnExec("yarn", ["configure"], opts);
    })
    .command("build", "build from source", (yargs) => {
    }, (argv) => {
        run("cmake-js");
    })
    .option("build-type", {
        choices: ["Release", "Debug"],
        default: "Release"
    })
    .demandCommand()
    .help()
    .argv;
