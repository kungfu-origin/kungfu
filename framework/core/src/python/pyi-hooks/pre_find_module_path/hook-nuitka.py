from PyInstaller.utils.hooks import logger


def pre_find_module_path(api):
    logger.info("Running pre-find module path hook for nuitka.")
