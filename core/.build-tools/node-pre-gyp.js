const {spawn} = require("child_process");
const path = require("path");
const semver = require("semver");

function run(cmd, args, title) {
    const child = spawn(cmd, args, {
        shell: true,
        stdio: ["inherit", "inherit", "inherit"],
        windowsHide: true
    });

    child.on("error", (error) => {
        process.stderr.write(error);
    });

    child.on("close", code => {
        console.log(`${title} exited with code ${code}`);
    });
}

function node_pre_gyp(cmd, argv) {
    const build_type_arg = argv.buildType === "Debug" ? ["--debug"] : [];
    const yarn_args = ["node-pre-gyp", ...build_type_arg, ...cmd];
    run("yarn", yarn_args, "node-pre-gyp install");
}

function aws_publish() {
    const config = path.resolve(path.join(path.dirname(__dirname),"package.json"));
    const version = require(config).version;
    const major = semver.major(version);
    const aws_args = ["s3", "cp", "build/stage/**/kungfu-*.tar.gz", `s3://kungfu/core/v${major}/${version}`];
    run("aws", aws_args, "aws publish");
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
    .command("publish", "publish to AWS S3", (yargs) => {
    }, (argv) => {
        node_pre_gyp(["package"], argv);
        aws_publish();
    })
    .option("build-type", {
        choices: ["Release", "Debug"],
        default: "Release"
    })
    .demandCommand()
    .help()
    .argv;
