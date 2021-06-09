#!/usr/bin/env node

const {spawnSync} = require("child_process");

const PyPIConfigName = "@kungfu-trader/kungfu-core:pypi";
const PrebuiltConfigSuffix = "binary_host_mirror"

const PyPI_US = "https://pypi.python.org/simple";
const PyPI_CN = "http://mirrors.aliyun.com/pypi/simple";

const PrebuiltHost_CN = "https://kungfu-prebuilt.s3.cn-northwest-1.amazonaws.com.cn";
const PrebuiltHost_US = "https://kungfu-prebuilt.s3.us-east-1.amazonaws.com";

const spawnOptsInherit = {shell: true, stdio: "inherit", windowsHide: true};

const argv = require("yargs/yargs")(process.argv.slice(2))
    .command("set-source-urls", "Set source URLs from where to get prebuilt/packages", (yargs) => {
        yargs.option("name", {
            type: "string",
            default: "kungfu",
            describe: "name of the prebuilt"
        });
    }, (argv) => {
        const buildingInUS = "GITHUB_WORKFLOW" in process.env;
        const hostConfigName = `${argv.name}_${PrebuiltConfigSuffix}`;
        const pypi = buildingInUS ? PyPI_US : PyPI_CN;
        const prebuiltHost = buildingInUS ? PrebuiltHost_US : PrebuiltHost_CN;
        const configure = (key, value) => {
            const npmArgs = ["config", "set", key, value];
            console.log(`$ npm ${npmArgs.join(' ')}`);
            spawnSync("npm", npmArgs);
        };
        configure(PyPIConfigName, pypi);
        configure(hostConfigName, prebuiltHost);
    })
    .command("show-source-urls", "Show source URLs from where to get prebuilt/packages", (yargs) => {
        yargs.option("name", {
            type: "string",
            default: "kungfu",
            describe: "name of the prebuilt"
        });
    }, (argv) => {
        const show = (configName) => {
            process.stdout.write(`${configName} = `);
            spawnSync("npm", ["config", "get", configName], spawnOptsInherit);
        };
        show(PyPIConfigName);
        show(`${argv.name}_${PrebuiltConfigSuffix}`);
    })
    .demandCommand()
    .help()
    .argv;