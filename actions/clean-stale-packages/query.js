const fetch = require("node-fetch");
const fs = require("fs");
const glob = require("glob");
const path = require("path");

const endpoint = "https://api.github.com/graphql"

function deletePackage(token, packageVersionId) {
    const query = `
      mutation {
        deletePackageVersion(input: { packageVersionId: "${packageVersionId}" }) { success }
     }`;

    fetch(endpoint, {
        method: "POST",
        body: JSON.stringify({query}),
        headers: {
            "Authorization": `Bearer ${token}`,
            "Accept": "application/vnd.github.package-deletes-preview+json",
        },
    }).then(res => res.text()).then(body => {
        const result = JSON.parse(body);
        console.log(result);
    }).catch(error => console.error(error));
}

exports.getPackage = function (owner, token, repo, actionPath) {
    const real_path = fs.realpathSync(path.resolve(actionPath));
    glob.sync(path.join(real_path, "*", "package.json")).map(p => {
        const config = require(p);
        const packageName = config.name.replace(`@${owner}/`, '');

        const query = `
          query {
          repository(name: "${repo}", owner: "${owner}") {
            packages(names: "${packageName}", last:1) {
              nodes {
                id
                name
                packageType
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

        fetch(endpoint, {
            method: "POST",
            body: JSON.stringify({query}),
            headers: {
                "Authorization": `Bearer ${token}`,
            },
        }).then(res => res.text()).then(body => {
            const result = JSON.parse(body);
            const packages = result.data.repository.packages.nodes;
            if (packages && packages.length) {
                const versions = packages[0].versions.nodes;
                if (versions && versions.length) {
                    if (config.version === versions[0].version) {
                        console.log(`Deleting published package: ${packageName}@${versions[0].version}`);
                        deletePackage(token, versions[0]);
                    } else {
                        console.log(`Updating package ${packageName}@${versions[0].version} to ${config.version}`);
                    }
                }
            }
        }).catch(error => console.error(error));
    });
}