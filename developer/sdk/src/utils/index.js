const path = require('path');

const isProduction = () => process.env.NODE_ENV === 'production';

const getResourcesKfcDir = () => {
  return path.join(__dirname, '..', '..', '..', 'kfc');
};

const customResolve = (path) => {
  if (isProduction()) {
    // eslint-disable-next-line no-undef
    return __non_webpack_require__.resolve(path);
  } else {
    return require.resolve(path);
  }
};

module.exports = { isProduction, getResourcesKfcDir, customResolve };
