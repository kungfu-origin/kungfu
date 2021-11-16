const path = require('path');
const app = require('@kungfu-trader/kungfu-app');
const distDir = path.join(path.dirname(__dirname), 'dist');
app.webpack(distDir, 'app');
