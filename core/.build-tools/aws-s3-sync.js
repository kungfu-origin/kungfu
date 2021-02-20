const {spawnSync} = require("child_process");

function run(retry_times) {
    const aws_args = ["s3", "sync", "build/stage/core", "s3://kungfu/core", "--acl", "public-read", "--only-show-errors"];

    console.log(`$ aws ${aws_args.join(' ')}`);

    const result = spawnSync("aws", aws_args, {
        shell: true,
        stdio: ["inherit", "inherit", "inherit"],
        windowsHide: true
    });

    if (result.status !== 0 && retry_times > 0) {
        console.log(`aws exited with code ${result.status}, retrying... (remaining ${retry_times - 1} times)`);
        run(retry_times - 1);
    } else {
        process.exit(result.status);
    }
}

run(5);