const {spawnSync} = require("child_process");
const os = require("os");

require("yargs/yargs")(process.argv.slice(2))
    .command("traceroute", "traceroute to AWS S3", (yargs) => {
    }, (argv) => {
        const traceroute = os.platform() === "win32" ? "tracert" : "traceroute";
        const result = spawnSync(traceroute, ["kungfu.s3.cn-northwest-1.amazonaws.com.cn"], {shell: true, stdio: "inherit", windowsHide: true});
        process.exit(result.status);
    })
    .demandCommand()
    .help()
    .argv;
