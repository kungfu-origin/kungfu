module.exports = function () {
  const binding = (() => {
    try {
      const moduleName = '@kungfu-trader/kungfu-core';
      const config = require(`${moduleName}/package.json`);
      const binary = config.binary;
      const kfcDir =
        process.env.KFC_DIR || `${moduleName}/${binary.module_path}`;

      const nodeBinding = require.resolve(
        `${kfcDir}/${binary.module_name}.node`,
      );
      const electronBinding = nodeBinding.replace('_node.', '_electron.');
      const kfcBinding = nodeBinding.replace('_node.', '_kfc.');
      const useElectron =
        process.platform !== 'win32' && 'electron' in process.versions;
      const useKfc =
        process.platform === 'linux' && process.env.KFC_AS_VARIANT === 'node';
      const binding_path = useElectron
        ? electronBinding
        : useKfc
        ? kfcBinding
        : nodeBinding;
      return require(binding_path);
    } catch (e) {
      console.error(`Can not find kungfu node binding: ${e}`);
      return {};
    }
  })();

  return {
    _binding: binding,
    longfist: binding.longfist,
    hash: binding.hash,
    formatTime: binding.formatTime,
    formatStringToHashHex: binding.formatStringToHashHex,
    parseTime: binding.parseTime,
    pyExec: binding.pyExec,
    pyEval: binding.pyEval,
    pyEvalFile: binding.pyEvalFile,
    shutdown: binding.shutdown,
    Assemble: function (arg) {
      if (Array.isArray(arg)) {
        return new binding.Assemble(
          arg.map(function (home) {
            return home;
          }),
        );
      } else {
        return new binding.Assemble([arg]);
      }
    },
    History: function (home) {
      return new binding.History(home);
    },
    ConfigStore: function (home) {
      return new binding.ConfigStore(home);
    },
    RiskSettingStore: function (home) {
      return new binding.RiskSettingStore(locator(home));
    },
    CommissionStore: function (home) {
      return new binding.CommissionStore(home);
    },
    watcher: function (
      home,
      name,
      bypassQuotes = false,
      bypassRestore = false,
    ) {
      return new binding.Watcher(home, name, bypassQuotes, bypassRestore);
    },
  };
};
