const path = require("path");
const app = require('@kungfu-trader/kungfu-app');
const distDir = path.join(process.cwd(), 'dist');

exports.build = function() {
  app.webpack(distDir, 'app');
};

exports.dev = function() {
  app.dev(distDir, 'app');
}
