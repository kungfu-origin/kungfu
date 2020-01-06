const bindings = exports._bindings = require('bindings')('node_yjj.node');
const path = require("path");

const layout_dir_from_home = function(home, category, group, name, mode, layout) {
    return path.join(home, category, group, name, layout, mode);
};

exports.locator = function(home) {
    return new bindings.Locator({
        has_env: function(name) {
            return false;
        },
        get_env: function(name) {
            return "env";
        },
        layout_dir: function(category, group, name, mode, layout) {
            return layout_dir_from_home(home, category, group, name, mode, layout);
        },
        layout_file: function(category, group, name, mode, layout, file_name) {
            return path.join(layout_dir_from_home(home, category, group, name, mode, layout), file_name + "." + layout);
        },
        list_page_id: function(category, group, name, mode) {
            return [1,2,3];
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
