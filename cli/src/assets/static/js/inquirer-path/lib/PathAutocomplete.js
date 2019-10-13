'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _fs = require('fs');

var _fs2 = _interopRequireDefault(_fs);

var _path = require('path');

var _path2 = _interopRequireDefault(_path);

var _Path = require('./Path');

var _Path2 = _interopRequireDefault(_Path);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

/**
 * State machine for a path autocomplete UI.
 * @param {string} cwd - The working directory from which path should be resolved
 * @param {boolean} [directoryOnly=false] - If set to should handle only directories
 */
var PathAutocomplete = function () {
  function PathAutocomplete(cwd) {
    var directoryOnly = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : false;

    _classCallCheck(this, PathAutocomplete);

    this.cwd = new _Path2.default(cwd);
    if (!this.cwd.isExistingDirectory()) {
      throw new Error('The provided working directory ' + cwd + ' does not exist or is not a directory.');
    }
    this.path = this.cwd;
    this.matches = null;
    this.matchIndex = -1;
    this.directoryOnly = directoryOnly;
  }

  /**
   * Reset the state of the class
   */


  _createClass(PathAutocomplete, [{
    key: 'reset',
    value: function reset() {
      this.path = this.cwd;
      this.matches = null;
      this.matchIndex = -1;
    }

    /**
     * @return The current input path
     */

  }, {
    key: 'getPath',
    value: function getPath() {
      return this.path;
    }

    /**
     * @return The currently selected path or the path
     */

  }, {
    key: 'getActivePath',
    value: function getActivePath() {
      if (this.matches == null) {
        return this.path;
      }
      var match = this.matches[this.matchIndex];
      if (match == null) {
        return this.path;
      }
      return match;
    }

    /**
     * @return True if the autocomplete is for directory only
     */

  }, {
    key: 'isDirectoryOnly',
    value: function isDirectoryOnly() {
      return this.directoryOnly;
    }

    /**
     * @return The current matches
     */

  }, {
    key: 'getMatches',
    value: function getMatches() {
      return this.matches;
    }

    /**
     * @returns The current match index or -1 if no match is selected;
     */

  }, {
    key: 'getMatchIndex',
    value: function getMatchIndex() {
      return this.matchIndex;
    }

    /**
     * @returns The current working directory
     */

  }, {
    key: 'getWorkingDirectory',
    value: function getWorkingDirectory() {
      return this.cwd;
    }

    /**
     * Set the path based on the user input
     * @param input
     */

  }, {
    key: 'setPath',
    value: function setPath(input) {
      var inputPath = new _Path2.default(this.cwd, input);
      if (this.path.equals(inputPath)) {
        return;
      }
      this.path = inputPath;
      this.matches = null;
      this.matchIndex = -1;
    }

    /**
     * Reset the matches/match index
     */

  }, {
    key: 'cancelMatch',
    value: function cancelMatch() {
      this.matches = null;
      this.matchIndex = -1;
    }

    /**
     * Use the current match index (or the provided one) to update the
     * current path
     * @param [matchIndex=this.matchSelectionIndex]
     */

  }, {
    key: 'selectMatch',
    value: function selectMatch(matchIndex) {
      if (!this.matches) {
        throw new Error('There are not potential matches');
      }
      var index = matchIndex == null ? this.matchIndex : matchIndex;
      if (index == null) {
        throw new Error('The match index is null');
      }
      var match = this.matches[index];
      if (match == null) {
        throw new Error('The match index does not exist');
      }
      this.path = match;
      this.matches = null;
      this.matchIndex = -1;
    }

    /**
     * Go to the next potential path. If there is only one match, the current path is set to
     * that match.
     * @param forward - If true, move to the next match. Otherwise move to the previous match.
     */

  }, {
    key: 'nextMatch',
    value: function nextMatch() {
      var forward = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : true;

      var matchesExist = !!this.matches;
      this.refreshMatches();
      if (!this.matches) {
        return;
      }
      if (this.matches.length === 1) {
        this.selectMatch(0);
        return;
      }
      // If the matches were inactive, do not update the selection yet
      if (!matchesExist) {
        return;
      }
      // Update the selection index while making sure that it is
      // a valid index within the matches length
      var position = this.matchIndex + (forward ? 1 : -1);
      var boundedPosition = position % this.matches.length;
      this.matchIndex = position >= 0 ? boundedPosition : this.matches.length + boundedPosition;
    }

    /**
     * Refresh the matches for the current path. NOOP if the matches have already been
     * computed
     */

  }, {
    key: 'refreshMatches',
    value: function refreshMatches() {
      if (this.matches !== null) {
        return;
      }
      this.matches = this.findMatches();
    }

    /**
     * Find a list of existing file system files or directory matching the current
     * input path. If multiple files/directories share the same prefix, a single entry
     * with the shared prefix is returned.
     */

  }, {
    key: 'findMatches',
    value: function findMatches() {
      var _this = this;

      var directory = this.path.getDirectory();
      if (!directory.isExistingDirectory()) {
        return null;
      }
      // For a directory, no need to filter on a prefix.
      var prefix = this.path.isDirectory() ? '' : this.path.getBaseName();
      var matches = _fs2.default.readdirSync(directory.getAbsolutePath())
      // Filter on the base input name
      .filter(function (child) {
        return child.startsWith(prefix);
      })
      // Map to Path instances
      .map(function (name) {
        var childPath = directory.getRelativePath() + name;
        try {
          if (_fs2.default.statSync(childPath).isDirectory()) {
            return new _Path2.default(_this.cwd, childPath + _path2.default.sep);
          }
        } catch (ex) {
          // Be more selective
        }
        return new _Path2.default(_this.cwd, childPath);
      })
      // Get rid of non directory if the ShellPath handles directories only
      .filter(function (child) {
        return !_this.directoryOnly || child.isExistingDirectory();
      })
      // Sort by directory/file and then name
      .sort(function (child1, child2) {
        if (child1.isDirectory() !== child2.isDirectory()) {
          return child1.isExistingDirectory() ? 1 : -1;
        }
        return child1.getAbsolutePath().localeCompare(child2.getAbsolutePath());
      });
      if (matches.length === 0) {
        return null;
      }
      if (matches.length === 1) {
        return matches;
      }
      var commonMatch = PathAutocomplete.findCommonMatch(matches);
      if (commonMatch == null) {
        return matches;
      }
      return !commonMatch.equals(this.path) ? [commonMatch] : matches;
    }

    /**
     * Find a common prefix for the provided paths
     * @private
     * @returns {Path} A common path to all the potential paths
     */

  }], [{
    key: 'findCommonMatch',
    value: function findCommonMatch(matches) {
      // Sort the paths. The 2 extremes are the most different.
      var sortedName = matches.map(function (p) {
        return p.getRelativePath();
      }).sort();
      var a1 = sortedName[0];
      var a2 = sortedName[sortedName.length - 1];
      // Find the common prefix between a1 and a2.
      var i = 0;
      while (i < a1.length && a1.charAt(i) === a2.charAt(i)) {
        i += 1;
      }
      var prefix = a1.substr(0, i);
      if (i === 0) {
        return null;
      }
      return new _Path2.default(matches[0].getCurrentWorkingDirectory(), prefix);
    }
  }]);

  return PathAutocomplete;
}();

exports.default = PathAutocomplete;