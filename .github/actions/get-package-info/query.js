const fetch = require("node-fetch");

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

exports.getPackage = function (token, owner, repo, packageName) {
    const query = `
      query {
      repository(name: "${repo}", owner: "${owner}") {
        packages(names: "${packageName}", first: 1) {
          nodes {
            id
            name
            packageType
            versions(first: 1) {
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
        const versions = packages[0].versions.nodes;
        console.log(versions);
        deletePackage(token, versions[0]);
    }).catch(error => console.error(error));
}