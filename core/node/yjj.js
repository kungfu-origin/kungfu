const bindings = exports._bindings = require('bindings')('node_yjj.node');
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

exports.locator = function(home) {
    return new bindings.Locator({
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
    });
};

exports.io_device = function(mode, category, group, name, locator) {
    return new bindings.IODevice(mode, category, group, name, locator);
};

exports.app = function(mode, category, group, name, locator) {
    return new bindings.Apprentice(mode, category, group, name, locator);
};
