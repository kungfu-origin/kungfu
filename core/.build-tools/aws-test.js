const {spawnSync} = require("child_process");
const os = require("os");

const spawnOpts = {shell: true, stdio: "inherit", windowsHide: true};

require("yargs/yargs")(process.argv.slice(2))
    .command("traceroute", "traceroute to AWS S3", (yargs) => {
    }, (argv) => {
        const traceroute = os.platform() === "win32" ? "tracert" : "traceroute";
        if (os.platform() === "linux") {
            spawnSync("apt-get", ["install", "-y", "traceroute"], spawnOpts);;
        }
        spawnSync(traceroute, ["kungfu.s3.cn-northwest-1.amazonaws.com.cn"], spawnOpts);
    })
    .demandCommand()
    .help()
    .argv;
