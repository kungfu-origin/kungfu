const path = require("path");
const glob = require("glob");
const github = require("@actions/github");

const deletePackage = async function (graphql, packageVersionId) {
    const query = `mutation { deletePackageVersion(input: { packageVersionId: "${packageVersionId}" }) { success }}`;
    const result = await graphql(query);
    console.log(result);
}

exports.cleanStalePackages = function (owner, token, repo, actionPath) {
    const octokit = github.getOctokit(token);
    const graphql = octokit.graphql.defaults({
        headers: {
            "Accept": "application/vnd.github.package-deletes-preview+json",
        }
    });
    const checkPackage = async function (config) {
        const packageName = config.name.replace(`@${owner}/`, '');
        const query = `
          query {
          repository(name: "${repo}", owner: "${owner}") {
            packages(names: "${packageName}", last:1) {
              nodes {
                versions(last: 1) {
                  nodes {
                    id
                    version
                  }
                }
              }
            }
          }
        }`;

        const {repository} = await graphql(query);
        const packages = repository.packages.nodes;
        if (packages && packages.length) {
            const versions = packages[0].versions.nodes;
            if (versions && versions.length) {
                if (config.version === versions[0].version) {
                    console.log(`Deleting stale package: ${packageName}@${versions[0].version} ...`);
                    deletePackage(graphql, versions[0].id).catch(error => {
                        console.error(`Failed to delete ${packageName}@${versions[0].version}. ${error}`);
                    });
                } else {
                    console.log(`Updating package ${packageName}@${versions[0].version} to ${config.version}`);
                }
            }
        }
    }
    glob.sync(path.join(path.resolve(actionPath), "*", "package.json")).map(p => {
        const config = require(p);
        checkPackage(config).catch(error => {
            console.error(`Package ${config.name}@${config.version} got ${error}`);
        });
    });
}