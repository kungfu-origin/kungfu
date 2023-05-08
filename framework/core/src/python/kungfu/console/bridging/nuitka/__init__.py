#  SPDX-License-Identifier: Apache-2.0

import kungfu
import os
import sys
import subprocess

from kungfu.console import site, variants
from os.path import abspath, dirname


def disableStaticallyLinkedPython():
    from nuitka import PythonVersions

    PythonVersions.isStaticallyLinkedPython = lambda: False


def useEngagedCommands():
    from nuitka.build import DataComposerInterface, SconsInterface
    from nuitka.utils.Execution import withEnvironmentVarsOverridden

    def runEngagedDataComposer(source_dir):
        mapping = {
            "NUITKA_PACKAGE_HOME": dirname(abspath(sys.modules["nuitka"].__path__[0])),
            "PATH": os.environ["PATH"],
            "PYTHONPATH": os.pathsep.join(sys.path),
        }
        blob_filename = DataComposerInterface.getConstantBlobFilename(source_dir)
        with withEnvironmentVarsOverridden(mapping):
            subprocess.check_call(
                [
                    sys.executable,
                    "-m",
                    "kungfu",
                    "engage",
                    "nuitka-data-composer",
                    source_dir,
                    blob_filename,
                ],
                shell=False,
            )
        return blob_filename

    def getEngagedSconsBinaryCall():
        os.environ["PYTHONPATH"] = (
            dirname(dirname(kungfu.__file__))
            + os.pathsep
            + dirname(kungfu.__binding__.__file__)
        )
        return [sys.executable, "-m", "kungfu", "engage", "scons"]

    DataComposerInterface.runDataComposer = runEngagedDataComposer
    SconsInterface._getSconsBinaryCall = getEngagedSconsBinaryCall


def useEngagedEnvironment():
    from nuitka.build import SconsUtils

    def withKungfuLib():
        origin = SconsUtils.createEnvironment

        def createEnvironment(**kwargs):
            env = origin(**kwargs)
            env.Append(LIBPATH=dirname(kungfu.__binding__.__file__))
            return env

        return createEnvironment

    SconsUtils.createEnvironment = withKungfuLib()


def parseOptions():
    from nuitka import Options

    Options.parseArgs()
    Options.commentArgs()


def loadPlugins():
    from nuitka.plugins import Plugins
    from nuitka.plugins.standard import (
        AntiBloatPlugin,
        ConsiderPyLintAnnotationsPlugin,
        DataFilesPlugin,
        DelvewheelPlugin,
        DillPlugin,
        EnumPlugin,
        EventletPlugin,
        GeventPlugin,
        GlfwPlugin,
        ImplicitImports,
        KivyPlugin,
        MatplotlibPlugin,
        MultiprocessingPlugin,
        NumpyPlugin,
        OptionsNannyPlugin,
        PbrPlugin,
        PkgResourcesPlugin,
        PmwPlugin,
        PySidePyQtPlugin,
        PywebViewPlugin,
        TensorflowPlugin,
        TkinterPlugin,
        TorchPlugin,
        UpxPlugin,
    )

    for plugin_module in [
        AntiBloatPlugin,
        ConsiderPyLintAnnotationsPlugin,
        DataFilesPlugin,
        DelvewheelPlugin,
        DillPlugin,
        EnumPlugin,
        EventletPlugin,
        GeventPlugin,
        GlfwPlugin,
        ImplicitImports,
        KivyPlugin,
        MatplotlibPlugin,
        MultiprocessingPlugin,
        NumpyPlugin,
        OptionsNannyPlugin,
        PbrPlugin,
        PkgResourcesPlugin,
        PmwPlugin,
        PySidePyQtPlugin,
        PywebViewPlugin,
        TensorflowPlugin,
        TkinterPlugin,
        TorchPlugin,
        UpxPlugin,
    ]:
        plugin_classes = set(
            obj
            for obj in plugin_module.__dict__.values()
            if Plugins.isObjectAUserPluginBaseClass(obj)
        )

        detectors = [
            plugin_class
            for plugin_class in plugin_classes
            if hasattr(plugin_class, "detector_for")
        ]

        for detector in detectors:
            plugin_class = detector.detector_for
            assert detector.plugin_name is None, detector
            detector.plugin_name = plugin_class.plugin_name

            if plugin_class not in plugin_classes:
                Plugins.plugins_logger.sysexit(
                    "Plugin detector %r references unknown plugin %r"
                    % (detector, plugin_class)
                )

            plugin_classes.remove(detector)
            plugin_classes.remove(plugin_class)

            Plugins.plugin_name2plugin_classes[plugin_class.plugin_name] = (
                plugin_class,
                detector,
            )

        for plugin_class in plugin_classes:
            Plugins.plugin_name2plugin_classes[plugin_class.plugin_name] = (
                plugin_class,
                None,
            )
    Plugins.loadStandardPluginClasses = lambda: True


def patchImportHardNodes():
    from nuitka.importing import Importing
    from nuitka.nodes import ImportHardNodes
    from nuitka.nodes import ExpressionBases
    from nuitka.utils import ModuleNames

    class ExpressionImportHardBase(ImportHardNodes.ExpressionBase):
        # pylint: disable=abstract-method
        __slots__ = ("module_name", "finding", "module_kind", "module_filename")

        def __init__(self, module_name, source_ref):
            ExpressionBases.ExpressionBase.__init__(self, source_ref)

            self.module_name = ModuleNames.ModuleName(module_name)

            self.finding = None
            self.module_filename = None

            (
                _module_name,
                self.module_filename,
                self.module_kind,
                self.finding,
            ) = Importing.locateModule(
                module_name=self.module_name,
                parent_package=None,
                level=0,
            )

        def getModulesUsageAttempt(self):
            return Importing.makeModuleUsageAttempt(
                module_name=self.module_name,
                filename=self.module_filename,
                module_kind=self.module_kind,
                finding=self.finding,
                level=0,
                source_ref=self.source_ref,
            )

    ImportHardNodes.ExpressionImportHardBase = ExpressionImportHardBase


def setup():
    site.setup()
    disableStaticallyLinkedPython()
    useEngagedCommands()
    useEngagedEnvironment()
    os.environ.update(
        {
            "PYTHONPATH": os.pathsep.join(sys.path),
        }
    )
    variants.enable("python")


def main():
    loadPlugins()
    parseOptions()
    patchImportHardNodes()

    from nuitka import MainControl

    MainControl.main()
