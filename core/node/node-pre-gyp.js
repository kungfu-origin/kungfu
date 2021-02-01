const {spawn} = require("child_process");
const argv = require("yargs/yargs")(process.argv.slice(2))
    .command("install", "install through node-pre-gyp", (yargs) => {
        yargs.positional("build-type", {
            choices: ["Release", "Debug"],
            default: "Release"
        })
    }, (argv) => {
        const build_type_arg = argv.buildType === "Debug" ? ["--debug"] : []
        const yarn_args = ["node-pre-gyp", "install", "--fallback-to-build", ...build_type_arg]

        const install = spawn("yarn", yarn_args, {
            shell: true,
            stdio: ["inherit", "inherit", "inherit"],
            windowsHide: true
        });

        install.on("error", (error) => {
            process.stderr.write(error)
        });

        install.on("close", code => {
            console.log(`yarn node-pre-gyp install exited with code ${code}`);
        });
    })
    .demandCommand()
    .help()
    .argv
