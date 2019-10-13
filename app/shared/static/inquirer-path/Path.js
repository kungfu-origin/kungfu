'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _fs = require('fs');

var _fs2 = _interopRequireDefault(_fs);

var _path = require('path');

var _path2 = _interopRequireDefault(_path);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

/**
 * A system path that may or may not exist. If the path points to
 * a directory, it should end with path.sep.
 * @param cwd - The working directory from which the path should be resolved.
 * @param relativePath - The path relative to the current working directory.
 */
var Path = function () {
  function Path(cwd) {
    var relativePath = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : '';

    _classCallCheck(this, Path);

    this.cwd = cwd instanceof Path ? cwd.getAbsolutePath() : cwd;
    this.relativePath = relativePath;
    this.absolutePath = _path2.default.resolve(this.cwd, relativePath);
  }

  /**
   * @returns {boolean} True if the path exists
   */


  _createClass(Path, [{
    key: 'exists',
    value: function exists() {
      try {
        _fs2.default.accessSync(this.absolutePath, _fs2.default.R_OK);
        return true;
      } catch (error) {
        return false;
      }
    }

    /**
     * @returns True if the path is an existing file
     */

  }, {
    key: 'isExistingFile',
    value: function isExistingFile() {
      try {
        return _fs2.default.statSync(this.absolutePath).isFile();
      } catch (error) {
        return false;
      }
    }

    /**
     * @returns True if the path is an existing directory
     */

  }, {
    key: 'isExistingDirectory',
    value: function isExistingDirectory() {
      try {
        return _fs2.default.statSync(this.absolutePath).isDirectory();
      } catch (error) {
        return false;
      }
    }

    /**
     * @returns True if the relative path ends with path.sep or it is not
     * present at all
     */

  }, {
    key: 'isDirectory',
    value: function isDirectory() {
      return this.relativePath.length === 0 || this.relativePath.endsWith(_path2.default.sep);
    }

    /**
     * @returns The current working directory path from which the
     * relative path is resolved.
     */

  }, {
    key: 'getCurrentWorkingDirectory',
    value: function getCurrentWorkingDirectory() {
      return this.cwd;
    }

    /**
     * @returns The absolute path of this instance
     */

  }, {
    key: 'getAbsolutePath',
    value: function getAbsolutePath() {
      return this.absolutePath;
    }

    /**
     * @returns The relative path of this instance
     */

  }, {
    key: 'getRelativePath',
    value: function getRelativePath() {
      return this.relativePath;
    }

    /**
     * @returns The base name of the current path
     */

  }, {
    key: 'getBaseName',
    value: function getBaseName() {
      return _path2.default.basename(this.absolutePath);
    }

    /**
     * @return The directory containing this path
     */

  }, {
    key: 'getDirectory',
    value: function getDirectory() {
      if (this.isDirectory()) {
        return this;
      }
      var dirname = _path2.default.dirname(this.relativePath);
      if (dirname === '.' && !this.relativePath.startsWith('.' + _path2.default.sep)) {
        return new Path(this.cwd);
      } else if (dirname.endsWith(_path2.default.sep)) {
        return new Path(this.cwd, dirname);
      }
      return new Path(this.cwd, dirname + _path2.default.sep);
    }

    /**
     * @param p The compared path
     * @return True if the paths are equal
     */

  }, {
    key: 'equals',
    value: function equals(p) {
      if (!p || !(p instanceof Path)) {
        return false;
      }
      return this.getCurrentWorkingDirectory() === this.getCurrentWorkingDirectory() && this.getRelativePath() === p.getRelativePath();
    }
  }]);

  return Path;
}();

exports.default = Path;