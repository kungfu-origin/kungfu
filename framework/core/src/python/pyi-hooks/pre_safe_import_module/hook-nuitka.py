from PyInstaller.utils.hooks import logger


def pre_safe_import_module(api):
    logger.info("Pre-safe import module hook for nuitka was executed.")
