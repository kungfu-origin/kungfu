'use strict';

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

var _runAsync = require('run-async');

var _runAsync2 = _interopRequireDefault(_runAsync);

var _base = require('inquirer/lib/prompts/base');

var _base2 = _interopRequireDefault(_base);

var _PathAutocomplete = require('./PathAutocomplete');

var _PathAutocomplete2 = _interopRequireDefault(_PathAutocomplete);

var _PathPromptRenderer = require('./PathPromptRenderer');

var _PathPromptRenderer2 = _interopRequireDefault(_PathPromptRenderer);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _toConsumableArray(arr) { if (Array.isArray(arr)) { for (var i = 0, arr2 = Array(arr.length); i < arr.length; i++) { arr2[i] = arr[i]; } return arr2; } else { return Array.from(arr); } }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var TAB_KEY = 'tab';
var ENTER_KEY = 'return';
var ESCAPE_KEY = 'escape';

/* eslint-disable no-undef */

/* eslint-enable no-undef */

/**
 * An Inquirer prompt for a one or more file system path. It supports autocompletion
 * similarly to zshell.
 * @param {object} question
 * @param {string} question.name The name to use when storing the answer in the answers hash.
 * @param {string} question.message The message to display when prompting the user for a path.
 * @param {string} [question.cwd=process.cwd()] The default working directory from which
 * relative paths are resolved. It is also the default value.
 * @param {string} [question.default=process.cwd()] Same as question.cwd
 * @param {boolean} [question.multi=false] If set to true, the user can enter multiple paths
 * @param {boolean} [question.directoryOnly=false] If set to true, the user can only enter paths to
 * directories
 * @param {function} [question.validate]  Receive the user input and should return true if the value
 * is valid or an error message (String) otherwise. If false is returned, a default error message is
 * provided. If question.multi is true, it is called for each path entered by the user.
 * @param {function} [question.validateMulti] If question.multi is set to true, it is called once
 * the question has been answered. It should return true if the value is valid or an error message
 * (String) otherwise.
 * @param {function} [question.filter] Receive the user input and return the filtered value to be
 * used inside the program. The value returned will be added to the Answers hash.
 * @param {function} [question.when] Receive the current user answers hash and should return true
 * or false depending on whether or not this question should be asked. The value can also be a
 * simple boolean..
 * @param rl An instance of readline.Interface
 * @param answers The answers provided by the user to other prompts
 */
var PathPrompt = function (_BasePrompt) {
  _inherits(PathPrompt, _BasePrompt);

  function PathPrompt(question, rl, answers) {
    _classCallCheck(this, PathPrompt);

    // bind event listeners
    var _this = _possibleConstructorReturn(this, (PathPrompt.__proto__ || Object.getPrototypeOf(PathPrompt)).call(this, question, rl, answers));

    _this.bindedOnExit = _this.onExit.bind(_this);
    _this.bindedOnKeyPress = _this.onKeyPress.bind(_this);

    _this.paths = [];
    _this.autocomplete = new _PathAutocomplete2.default(question.cwd || question.default || process.cwd(), question.directoryOnly);
    _this.renderer = new _PathPromptRenderer2.default(_this.rl, _this.screen, _this.autocomplete, _this.opt.message);
    _this.answerCallback = function () {};
    _this.isTryingExit = false;
    _this.listeners = {
      SIGINT: []
    };
    return _this;
  }

  /**
   * Runs the path prompt.
   * @param callback - Called when the prompt has been answered successfully
   * @returns
     */


  _createClass(PathPrompt, [{
    key: '_run',
    value: function _run(callback) {
      this.answerCallback = callback;
      // backup event listeners so we can rebind them later
      this.listeners.SIGINT = this.rl.listeners('SIGINT');
      this.rl.removeAllListeners('SIGINT');

      this.rl.addListener('SIGINT', this.bindedOnExit);
      this.rl.input.addListener('keypress', this.bindedOnKeyPress);
      this.renderer.render();
      return this;
    }

    /**
     * Handle the keyPress events and update the @{link PathAutocomplete} state
     * accordingly.
     * @param value The string value of the keyboard entry
     * @param key Information about the name of the key and whether other special
     * keys were pressed at the same time.
     */

  }, {
    key: 'onKeyPress',
    value: function onKeyPress(value, key) {
      if (key && key.ctrl) {
        return;
      }
      var keyName = key ? key.name : value;
      this.isTryingExit = false;
      switch (keyName) {
        case TAB_KEY:
          try {
            this.autocomplete.nextMatch(!key.shift);
            this.renderer.render();
          } catch (err) {
            this.renderer.renderError(err.message);
          }
          break;
        case ENTER_KEY:
          this.onEnterPressed();
          break;
        case ESCAPE_KEY:
          this.onEscapePressed();
          break;
        default:
          this.autocomplete.setPath(this.rl.line);
          this.renderer.render();
          break;
      }
    }

    /**
     * Select the current match or submit the answer
     */

  }, {
    key: 'onEnterPressed',
    value: function onEnterPressed() {
      if (this.autocomplete.getMatchIndex() !== -1) {
        this.autocomplete.selectMatch();
        this.renderer.render();
      } else {
        this.submitAnswer();
      }
    }

    /**
     * Cancel matching or submit the answer for a multi path prompt
     */

  }, {
    key: 'onEscapePressed',
    value: function onEscapePressed() {
      if (this.autocomplete.getMatchIndex() !== -1) {
        this.autocomplete.cancelMatch();
        this.renderer.render();
      } else if (this.opt.multi) {
        this.submitAnswer(true);
      }
    }

    /**
     * Event handler for cancel events (SIGINT). If the user is currently selecting a path,
     * it causes the selection to be cancelled. If the prompt is a multi path prompt, it
     * causes the question to be done. If none of these conditions are met, the event handlers
     * are cleaned up and the regular SIGINT handlers are involved
     */

  }, {
    key: 'onExit',
    value: function onExit() {
      for (var _len = arguments.length, args = Array(_len), _key = 0; _key < _len; _key++) {
        args[_key] = arguments[_key];
      }

      // Cancel the path selection
      if (this.autocomplete.getMatchIndex() !== -1) {
        this.autocomplete.cancelMatch();
        this.renderer.render();
      } else if (this.opt.multi && !this.isTryingExit) {
        this.submitAnswer(true);
        this.isTryingExit = true;
      } else {
        // Exit out
        this.restoreEventHandlers();
        // Call the restored SIGINT callbacks manually
        this.rl.listeners('SIGINT').forEach(function (listener) {
          return listener.apply(undefined, _toConsumableArray(args));
        });
      }
    }

    /**
     * Validate the answer and kill the prompt if it's either a single path
     * prompt or a multiple path prompt and submitMulti is set to true.
     * @param submitMulti If set to true, submit all answers
     */

  }, {
    key: 'submitAnswer',
    value: function submitAnswer() {
      var _this2 = this;

      var submitMulti = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : false;

      var answer = this.opt.multi && submitMulti ? this.paths : this.autocomplete.getPath().getAbsolutePath();

      var validate = (0, _runAsync2.default)(this.opt.validate);
      var filter = (0, _runAsync2.default)(this.opt.filter);

      var promiseChain = validate(answer, this.answers, this.opt.multi ? this.paths : null).then(function (isValid) {
        if (isValid !== true) {
          throw isValid;
        }
        return answer;
      }).then(filter);

      if (!this.opt.multi) {
        // Render the final path
        promiseChain = promiseChain.then(function (finalAnswer) {
          _this2.renderer.render(finalAnswer);
          return finalAnswer;
        });
      } else if (this.opt.multi && !submitMulti) {
        // For a new path entry, render a fresh prompt
        promiseChain = promiseChain.then(function (finalAnswer) {
          // Update the array keeping track of the answers
          _this2.paths.push(finalAnswer);
          // Create a new autocomplete instance for the new prompt
          _this2.autocomplete = new _PathAutocomplete2.default(_this2.autocomplete.getWorkingDirectory().getAbsolutePath(), _this2.autocomplete.isDirectoryOnly());
          _this2.renderer.renderNewPrompt(finalAnswer, _this2.autocomplete);
          return finalAnswer;
        });
      }
      // Kill the prompt
      if (!this.opt.multi || submitMulti) {
        promiseChain = promiseChain.then(function (finalAnswer) {
          _this2.status = 'answered';
          _this2.renderer.kill();
          _this2.restoreEventHandlers();
          _this2.answerCallback(finalAnswer);
        });
      }
      promiseChain.catch(function (error) {
        return _this2.renderer.renderError(error);
      });
    }

    /**
     * Unregister the instance's event handlers and register global event
     * handlers ones that were temporarily removed.
     */

  }, {
    key: 'restoreEventHandlers',
    value: function restoreEventHandlers() {
      var _this3 = this;

      this.rl.removeListener('SIGINT', this.bindedOnExit);
      this.rl.input.removeListener('keypress', this.bindedOnKeyPress);
      Object.keys(this.listeners).forEach(function (eventName) {
        _this3.listeners[eventName].forEach(function (listener) {
          _this3.rl.addListener(eventName, listener);
        });
      });
    }
  }]);

  return PathPrompt;
}(_base2.default);

exports.default = PathPrompt;