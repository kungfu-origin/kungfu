const {spawn} = require("child_process");

function node_pre_gyp(cmd, argv) {
    const build_type_arg = argv.buildType === "Debug" ? ["--debug"] : []
    const yarn_args = ["node-pre-gyp", ...build_type_arg, ...cmd]

    const child = spawn("yarn", yarn_args, {
        shell: true,
        stdio: ["inherit", "inherit", "inherit"],
        windowsHide: true
    });

    child.on("error", (error) => {
        process.stderr.write(error)
    });

    child.on("close", code => {
        console.log(`yarn node-pre-gyp install exited with code ${code}`);
    });
}

const argv = require("yargs/yargs")(process.argv.slice(2))
    .command("install", "install through node-pre-gyp", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["install", "--fallback-to-build"], argv)
    })
    .command("build", "build from source", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["configure", "build", "package"], argv)
    })
    .option("build-type", {
        choices: ["Release", "Debug"],
        default: "Release"
    })
    .demandCommand()
    .help()
    .argv
