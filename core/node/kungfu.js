const bindings = exports._bindings = require('bindings')('node_kungfu.node');
const fse = require("fs-extra");
const path = require("path");
const glob = require("glob");

const hex = function(n){
    const ns = n.toString(16);
    return ns.length === 8 ? ns : Array(8 - ns.length).fill('0').reduce((a, b) => a + b) + ns;
};

const layout_dir_from_home = function(home, category, group, name, mode, layout) {
    const dir = path.join(home, category, group, name, layout, mode);
    if (!fse.existsSync(dir)) {
        fse.ensureDirSync(dir)
    }
    return dir;
};

exports.longfist = bindings.longfist;

exports.locator = function(home) {
    return {
        has_env: function(name) {
            return name in process.env;
        },
        get_env: function(name) {
            return process.env[name];
        },
        layout_dir: function(category, group, name, mode, layout) {
            return layout_dir_from_home(home, category, group, name, mode, layout);
        },
        layout_file: function(category, group, name, mode, layout, file_name) {
            return path.join(layout_dir_from_home(home, category, group, name, mode, layout), file_name + "." + layout);
        },
        list_page_id: function(category, group, name, mode, dest_id) {
            const suffix = ".journal";
            const dest_dir = layout_dir_from_home(home, category, group, name, mode, "journal");
            const file_re = path.join(dest_dir, hex(dest_id) + '.*' + suffix);
            const pages = glob.sync(file_re);
            return pages.map(p => parseInt(p.substr(dest_dir.length + 10, p.length - dest_dir.length - 10 - suffix.length)));
        },
        list_locations: function(category, group, name, mode) {
            return [];
        },
        list_location_dest: function(category, group, name, mode) {
            return [];
        }
    };
};

exports.io_device = function(mode, category, group, name, locator) {
    return new bindings.IODevice(mode, category, group, name, locator);
};

exports.watcher = function(locator, name) {
    let wb = new bindings.Watcher(locator, name);
    wb.run = function(stepCallback) {
        const self = this;
        const setTimerPromiseTask = function (fn, interval) {
            if (interval === void 0) { interval = 500; }
            var taskTimer = null;
            function timerPromiseTask(fn, interval) {
                if (interval === void 0) { interval = 500; }
                if (taskTimer)
                    clearTimeout(taskTimer);
                fn()
                    .finally(function () {
                        taskTimer = setTimeout(function () {
                            timerPromiseTask(fn, interval);
                        }, interval);
                    });
            }
            timerPromiseTask(fn, interval);
        };
        setTimerPromiseTask(() => {
            return new Promise((resolve) => {
                if (self.isUsable() && !self.isLive() && !self.isStarted()) {
                    self.setup();
                }
                if (self.isLive()) {
                    self.step();
                }
                if (self.isLive() && self.isStarted()) {
                    stepCallback(self);
                }
                resolve();
            })
        });
    };
    return wb;
};
