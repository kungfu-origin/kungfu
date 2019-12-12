function Options(options) {
    options = options || {};

    this._defaultOptions = {
        replaceNodeKill: true,
        warmUp: false
    };

    this._options = Object.assign({}, this._defaultOptions, options);
};

Options.prototype.get = function (id) {
    if (this._options.hasOwnProperty(id)) {
        return this._options[id];
    }

    return null;
}

module.exports = Options;
