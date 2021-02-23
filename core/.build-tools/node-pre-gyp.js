const {spawnSync} = require("child_process");
const fs = require("fs");
const path = require("path");

function run(cmd, args) {
    const real_cwd = fs.realpathSync(path.resolve(process.cwd()));
    console.log(`$ ${cmd} ${args.join(' ')}`);
    const result = spawnSync(cmd, args, {
        shell: true,
        stdio: ["inherit", "inherit", "inherit"],
        windowsHide: true,
        cwd: real_cwd
    });
    process.exit(result.status);
}

function node_pre_gyp(cmd, argv) {
    const build_type_arg = argv.buildType === "Debug" ? ["--debug"] : [];
    const yarn_cmd_arg = ["node-pre-gyp", ...build_type_arg, ...cmd];
    run("yarn", yarn_cmd_arg);
}

const argv = require("yargs/yargs")(process.argv.slice(2))
    .command("install", "install through node-pre-gyp", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["install", "--fallback-to-build"], argv);
    })
    .command("build", "build from source", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["configure", "build"], argv);
    })
    .command("package", "package binary", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["package"], argv);
    })
    .option("build-type", {
        choices: ["Release", "Debug"],
        default: "Release"
    })
    .demandCommand()
    .help()
    .argv;
