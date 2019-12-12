function Signals() {
    this._supportedSignals = {
        'SIGBREAK': 1,
        'SIGINT': 2
    }
}

Signals.prototype.isSupported = function (signal) {
    return this._supportedSignals.hasOwnProperty(signal);
}

Signals.prototype.idToNumber = function (signal) {
    if (this._supportedSignals.hasOwnProperty(signal)) {
        return this._supportedSignals[signal];
    }

    return null;
}

module.exports = Signals;
