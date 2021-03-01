#!/usr/bin/env node

const { spawnSync } = require("child_process");

const prebuilt_host_cn = "https://kungfu-prebuilt.s3.cn-northwest-1.amazonaws.com.cn";
const prebuilt_host_us = "https://kungfu-prebuilt.s3.us-west-1.amazonaws.com";

const spawnOptsInherit = { shell: true, stdio: "inherit", windowsHide: true };

require("yargs/yargs")(process.argv.slice(2))
    .command("set-prebuilt-host", "Set host for fetching prebuilt binaries", (yargs) => {
        yargs.option("name", {
            type: "string",
            default: "kungfu",
            describe: "name of the prebuilt"
        });
    }, (argv) => {
        const config_name = `${argv.name}_binary_host_mirror`;
        const prebuilt_host = "GITHUB_WORKFLOW" in process.env ? prebuilt_host_us : prebuilt_host_cn;
        const npm_args = ["config", "set", config_name, prebuilt_host];
        console.log(`$ npm ${npm_args.join(' ')}`);
        spawnSync("npm", npm_args);
    })
    .command("get-prebuilt-host", "Get host for fetching prebuilt binaries", (yargs) => {
        yargs.option("name", {
            type: "string",
            default: "kungfu",
            describe: "name of the prebuilt"
        });
    }, (argv) => {
        const config_name = `${argv.name}_binary_host_mirror`;
        process.stdout.write(`${config_name} = `);
        spawnSync("npm", ["config", "get", config_name], spawnOptsInherit);
    })
    .demandCommand()
    .help()
    .argv;