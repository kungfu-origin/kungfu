#!/usr/bin/env node

const fs = require("fs");
const os = require("os");
const path = require("path");
const {spawnSync} = require("child_process");

const PyPIConfigName = "@kungfu-trader/kungfu-core:pypi";
const PrebuiltModuleName = "kungfu_node";
const PrebuiltConfigSuffix = "binary_host_mirror"

const PyPI_US = "https://pypi.python.org/simple";
const PyPI_CN = "http://mirrors.aliyun.com/pypi/simple";

const PrebuiltHost_CN = "https://kungfu-prebuilt.s3.cn-northwest-1.amazonaws.com.cn";
const PrebuiltHost_US = "https://kungfu-prebuilt.s3.us-east-1.amazonaws.com";

const spawnOptsPipe = {shell: true, stdio: "pipe", windowsHide: true};
const spawnOptsInherit = {shell: true, stdio: "inherit", windowsHide: true};

function showSourceUrl(configName) {
    const result = spawnSync("npm", ["config", "get", `"${configName}"`], spawnOptsPipe);
    const output = result.output.filter(e => e && e.length > 0).toString().trimEnd();
    console.log(`${configName} = ${output}`);
}

function showSourceUrls(prebuiltName) {
    showSourceUrl(PyPIConfigName);
    showSourceUrl(`${prebuiltName}_${PrebuiltConfigSuffix}`);
}

exports.argv = require("yargs/yargs")(process.argv.slice(2))
    .command("set-source-urls", "Set source URLs from where to get prebuilt/packages", (yargs) => {
        yargs.option("name", {
            type: "string",
            default: PrebuiltModuleName,
            describe: "name of the prebuilt"
        });
    }, (argv) => {
        const buildingInUS = "GITHUB_ACTION" in process.env;
        const hostConfigName = `${argv.name}_${PrebuiltConfigSuffix}`;
        const pypi = buildingInUS ? PyPI_US : PyPI_CN;
        const prebuiltHost = buildingInUS ? PrebuiltHost_US : PrebuiltHost_CN;
        const configure = (key, value) => {
            const npmArgs = ["config", "set", key, value];
            console.log(`$ npm ${npmArgs.join(' ')}`);
            const result = spawnSync("npm", npmArgs, spawnOptsInherit);
            if (result.status !== 0) {
                console.error(`Failed with status ${status}`);
                process.exit(result.status);
            }
        };
        configure(PyPIConfigName, pypi);
        configure(hostConfigName, prebuiltHost);
        showSourceUrls(argv.name);
    })
    .command("show-source-urls", "Show source URLs from where to get prebuilt/packages", (yargs) => {
        yargs.option("name", {
            type: "string",
            default: PrebuiltModuleName,
            describe: "name of the prebuilt"
        });
    }, (argv) => {
        showSourceUrls(argv.name);
    })
    .command("dir", "Show kungfu core base directory", (yargs) => {
    }, (argv) => {
        console.log(fs.realpathSync(path.dirname(__dirname)));
    })
    .command("info", "Show kungfu core build info", (yargs) => {
    }, (argv) => {
        const buildinfoPath = path.join(path.dirname(__dirname), "build", "kfc", "kungfubuildinfo.json");
        if (fs.existsSync(buildinfoPath)) {
            const buildinfo = require(buildinfoPath);
            console.log(buildinfo);
        } else {
            console.warn(`Info file missing, ${buildinfoPath} not exists`);
        }
    })
    .demandCommand()
    .help()
    .argv;