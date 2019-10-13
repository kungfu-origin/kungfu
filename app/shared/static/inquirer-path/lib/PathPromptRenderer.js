'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _chalk = require('chalk');

var _chalk2 = _interopRequireDefault(_chalk);

var _path = require('path');

var _path2 = _interopRequireDefault(_path);

var _readline = require('readline');

var _readline2 = _interopRequireDefault(_readline);

var _screenManager = require('inquirer/lib/utils/screen-manager');

var _screenManager2 = _interopRequireDefault(_screenManager);

var _PathAutocomplete = require('./PathAutocomplete');

var _PathAutocomplete2 = _interopRequireDefault(_PathAutocomplete);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

var RANGE_SIZE = 5;

/**
 * Render the path prompt UI based on a instance of { @link PathAutocomplete }
 */

var PathPromptRenderer = function () {
  function PathPromptRenderer(rl, screen, autocomplete, message) {
    _classCallCheck(this, PathPromptRenderer);

    this.rl = rl;
    this.screen = screen;
    this.message = message;
    this.autocomplete = autocomplete;
  }

  /**
   * Restore the state of the resources used by the renderer
   */


  _createClass(PathPromptRenderer, [{
    key: 'kill',
    value: function kill() {
      this.screen.done();
    }

    /**
     * Render the prompt UI
     * @param [finalAnswer=null] If present, display the final answer
     */

  }, {
    key: 'render',
    value: function render(finalAnswer) {
      var message = this.buildMainContent(finalAnswer);
      var bottom = finalAnswer ? '' : this.buildBottomContent();
      this.screen.render(message, bottom);
      if (!finalAnswer) {
        // Reset the line value to match the state of the PathAutocomplete instance
        this.resetCursor();
      }
    }

    /**
     * Render the UI for a new prompt. It finalizes the current render,
     * inserts a new line and render a new path prompt.
     * @param finalAnswer
     * @param autocomplete The new autocomplete state instance
     */

  }, {
    key: 'renderNewPrompt',
    value: function renderNewPrompt(finalAnswer, autocomplete) {
      // Finalize the current render
      this.render(finalAnswer);
      // Add a new line to keep the rendered answer
      this.rl.output.unmute();
      this.rl.output.write('\n');
      this.rl.output.mute();
      this.autocomplete = autocomplete;
      // Render the new prompt
      this.render();
    }

    /**
     * Render the error UI
     * @param error
     */

  }, {
    key: 'renderError',
    value: function renderError(error) {
      this.screen.render(this.buildMainContent(), _chalk2.default.red('>> ') + error);
      this.resetCursor();
    }

    /**
     * Render the main content of the prompt. The message includes the question and
     * the current response.
     * @param [finalAnswer=null] If present, display the final answer
     */

  }, {
    key: 'buildMainContent',
    value: function buildMainContent(finalAnswer) {
      var message = _chalk2.default.green('?') + ' ' + _chalk2.default.bold(this.message) + _chalk2.default.reset(' ');
      message += _chalk2.default.dim('(' + this.autocomplete.getWorkingDirectory().getBaseName() + ') ');
      if (finalAnswer) {
        message += _chalk2.default.cyan(finalAnswer);
      } else {
        message += this.autocomplete.getActivePath().getRelativePath();
      }
      return message;
    }

    /**
     * Render the bottom content of the prompt. It displays the current
     * selection state of the {@link PathAutocomplete} instance
     * @returns {string}
     */

  }, {
    key: 'buildBottomContent',
    value: function buildBottomContent() {
      var matches = this.autocomplete.getMatches();
      if (matches == null) {
        return '';
      }
      var matchIndex = this.autocomplete.getMatchIndex();
      var match = matches[matchIndex];

      return PathPromptRenderer.shortenArray(matches, matchIndex, RANGE_SIZE).map(function (potentialPath) {
        var suffix = potentialPath.isDirectory() ? _path2.default.sep : '';
        if (potentialPath === match) {
          return _chalk2.default.black.bgWhite(potentialPath.getBaseName() + suffix);
        }
        var colorize = potentialPath.isDirectory() ? _chalk2.default.red : _chalk2.default.green;
        return colorize(potentialPath.getBaseName()) + suffix;
      }).join('\n');
    }

    /**
     * Reset the input cursor to the end of the line
     */

  }, {
    key: 'resetCursor',
    value: function resetCursor() {
      // Move the display cursor
      var activeEntry = this.autocomplete.getActivePath().getRelativePath();
      if (activeEntry === this.rl.line) {
        return;
      }
      var cursorPosition = (this.autocomplete.getWorkingDirectory().getBaseName().length + this.message.length + activeEntry.length + 6) % this.rl.output.columns;

      this.rl.line = activeEntry;
      this.rl.output.unmute();
      _readline2.default.cursorTo(this.rl.output, cursorPosition);
      this.rl.cursor = activeEntry.length;
      this.rl.output.mute();
    }

    /**
     * Slice an array around a specific item so that it contains a specific number of elements.
     * @param items - The array to shorten
     * @param itemIndex - The index of the item that should be included in the returned slice
     * @param size - The desired size of the array to be returned
     * @returns
     */

  }], [{
    key: 'shortenArray',
    value: function shortenArray(items, itemIndex, size) {
      var length = items.length;
      var min = itemIndex - Math.floor(size / 2);
      var max = itemIndex + Math.ceil(size / 2);
      if (min < 0) {
        max = Math.min(length, max - min);
        min = 0;
      } else if (max >= length) {
        min = Math.max(0, min - (max - length));
        max = length;
      }
      return items.slice(min, max);
    }
  }]);

  return PathPromptRenderer;
}();

exports.default = PathPromptRenderer;