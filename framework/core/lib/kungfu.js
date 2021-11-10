const fse = require("fs-extra");
const glob = require("glob");
const path = require("path");

const hex = function(n) {
  const ns = n.toString(16);
  return ns.length === 8
    ? ns
    : Array(8 - ns.length)
    .fill("0")
    .reduce((a, b) => a + b) + ns;
};

const layout_dir_from_home = function(home, category, group, name, mode, layout) {
  const dir = path.join(home, category, group, name, layout, mode);
  if (!fse.existsSync(dir)) {
    fse.ensureDirSync(dir);
  }
  return dir;
};

const locator = function(home) {
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
      const file_re = path.join(dest_dir, hex(dest_id) + ".*" + suffix);
      const pages = glob.sync(file_re);
      return pages.map((p) =>
        parseInt(p.substr(dest_dir.length + 10, p.length - dest_dir.length - 10 - suffix.length))
      );
    },
    list_locations: function(category, group, name, mode) {
      const locations = [];
      const search_path = path.join(home, category, group, name, "journal", mode);
      glob.sync(search_path).map((p) => {
        const dirs = p.split(path.sep);
        const m = dirs.slice(dirs.length - 5);
        locations.push({ category: m[0], group: m[1], name: m[2], mode: m[4] });
      });
      return locations;
    },
    list_location_dest: function(category, group, name, mode) {
      const dest_dir = layout_dir_from_home(home, category, group, name, mode, "journal");
      const pages = glob.sync(path.join(dest_dir, "*.journal"));
      const destObj = {};
      pages.map((p) => {
        const uid = p.match(/.\w+\.\d+\.journal/g)[0].substr(1, 9);
        destObj[uid] = uid;
      });
      return Object.keys(destObj).map((k) => parseInt(k, 16));
    }
  };
};

module.exports = function() {
  const binding = (() => {
    try {
      const moduleName = "@kungfu-trader/kungfu-core";
      const config = require(`${moduleName}/package.json`);

      const binary = config.binary;

      const kfcDir = process.env.KFC_DIR || `${moduleName}/${binary.module_path}`;
      const nodeBinding = require.resolve(`${kfcDir}/${binary.module_name}.node`);
      const electronBinding = nodeBinding.replace("_node.", "_electron.");
      const kfcBinding = nodeBinding.replace("_node.", "_kfc.");

      const useElectron = "electron" in process.versions;
      const useKfc = process.platform === "linux" && process.env.KFC_AS_VARIANT === "node";
      const binding_path = useElectron ? electronBinding : useKfc ? kfcBinding : nodeBinding;

      return require(binding_path);
    } catch (e) {
      console.error(`Can not find kungfu node binding: ${e}`);
      return {};
    }
  })();

  return {
    "_binding": binding,
    "longfist": binding.longfist,
    "formatTime": binding.formatTime,
    "formatStringToHashHex": binding.formatStringToHashHex,
    "parseTime": binding.parseTime,
    "pyExec": binding.pyExec,
    "pyEval": binding.pyEval,
    "pyEvalFile": binding.pyEvalFile,
    "Assemble": function(arg) {
      if (Array.isArray(arg)) {
        return new binding.Assemble(
          arg.map(function(home) {
            return locator(home);
          })
        );
      } else {
        return new binding.Assemble([locator(arg)]);
      }
    },
    "IODevice": function(category, group, name, mode, home) {
      return new binding.IODevice(category, group, name, mode, locator(home));
    },
    "History": function(home) {
      return new binding.History(locator(home));
    },
    "ConfigStore": function(home) {
      return new binding.ConfigStore(locator(home));
    },
    "CommissionStore": function(home) {
      return new binding.CommissionStore(locator(home));
    },
    "watcher": function(home, name, bypassQuotes = false, bypassRestore = false) {
      return new binding.Watcher(locator(home), name, bypassQuotes, bypassRestore);
    }
  };
};
