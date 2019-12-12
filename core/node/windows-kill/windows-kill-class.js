var path = require('path');

var binary = require('node-pre-gyp');

var bindingPath = binary.find(path.resolve(path.join(__dirname, '../package.json')));

var Options = require('./options');
var Signals = require('./signals');

function WindowsKillClass(options) {
    this._native = require(bindingPath);
    this._nodeKill = process.kill;

    this._options = new Options(options);
    this._signals = new Signals();

    this._applyOptions();
};

WindowsKillClass.prototype._applyOptions = function () {
    if (this._options.get('replaceNodeKill') === true) {
        this._replaceNodeKill();
    }

    if (this._options.get('warmUp') === true) {
        this._native.warmUp();
    }
}

WindowsKillClass.prototype._replaceNodeKill = function () {
    var self = this;
    process.kill = function (pid, signal) {
        self.kill(pid, signal);
    }
}

WindowsKillClass.prototype.kill = function (pid, signal) {
    if (this._signals.isSupported(signal)) {
        if (pid != (pid | 0)) {
            throw new TypeError('invalid pid.');
        }

        var error = this._native.send(pid, this._signals.idToNumber(signal));

        if (error) {
            throw new Error('windows-kill ' + error);
        }
    } else {
        this._nodeKill(pid, signal);
    }
};

module.exports = WindowsKillClass;
