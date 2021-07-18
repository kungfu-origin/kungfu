const {spawnSync} = require("child_process");
const fs = require("fs");
const glob = require("glob");
const path = require("path");

function run(cmd, argv, check=true) {
    const real_cwd = fs.realpathSync(path.resolve(process.cwd()));
    console.log(`$ ${cmd} ${argv.join(' ')}`);
    const result = spawnSync(cmd, argv, {
        shell: true,
        stdio: "inherit",
        windowsHide: true,
        cwd: real_cwd
    });
    if (check && result.status !== 0) {
        process.exit(result.status);
    }
}

function node_pre_gyp(cmd, argv, check=true) {
    const build_type_arg = argv.buildType === "Debug" ? ["--debug"] : [];
    const yarn_cmd_arg = ["node-pre-gyp", ...build_type_arg, ...cmd];
    run("yarn", yarn_cmd_arg, check);
}

exports.argv = require("yargs/yargs")(process.argv.slice(2))
    .command("install", "install through node-pre-gyp", (yargs) => {
    }, (argv) => {
        const skipBuild = "KF_SKIP_FALLBACK_BUILD" in process.env;
        node_pre_gyp(skipBuild ? ["install"] : ["install", "--fallback-to-build"], argv, !skipBuild);
    })
    .command("build", "build from source", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["configure", "build"], argv);
    })
    .command("package", "package binary", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["package"], argv);
        const prebuilt = glob.sync(path.join("build", "stage", "**", "*.tar.gz"))[0];
        const wheel = glob.sync(path.join("build", "python", "dist", "*.whl"))[0];
        if (prebuilt && wheel) {
            console.log(`$ cp ${wheel} ${path.dirname(prebuilt)}`);
            fs.copyFileSync(wheel, path.join(path.dirname(prebuilt), path.basename(wheel)));
        }
    })
    .option("build-type", {
        choices: ["Release", "Debug"],
        default: "Release"
    })
    .demandCommand()
    .help()
    .argv;
