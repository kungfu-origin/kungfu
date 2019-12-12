var os = require('os');

var WindowsKillClass = require('./windows-kill-class');
var windowsKill = null;

module.exports = function (options) {
    if (os.platform() === 'win32') {
        if (windowsKill === null) {
            windowsKill = new WindowsKillClass(options);
        }
        return function (pid, signal) {
            windowsKill.kill(pid, signal);
        };
    }

    return process.kill;
};
