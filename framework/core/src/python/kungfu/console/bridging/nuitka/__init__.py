import kungfu
import os
import sys
import subprocess

from kungfu.console import variants
from os.path import abspath, dirname


def disableStaticallyLinkedPython():
    from nuitka import PythonVersions

    PythonVersions.isStaticallyLinkedPython = lambda: False


def useEngagedCommands():
    from nuitka.build import DataComposerInterface, SconsInterface

    def runEngagedDataComposer(source_dir):
        mapping = {
            "NUITKA_PACKAGE_HOME": dirname(abspath(sys.modules["nuitka"].__path__[0])),
            "PATH": os.environ["PATH"],
            "PYTHONPATH": os.pathsep.join(sys.path),
        }
        blob_filename = DataComposerInterface.getConstantBlobFilename(source_dir)
        with DataComposerInterface.withEnvironmentVarsOverriden(mapping):
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


def loadPlugins():
    from nuitka.plugins import Plugins
    from nuitka.plugins.standard import (
        AntiBloat,
        ConsiderPyLintAnnotationsPlugin,
        DataFileCollectorPlugin,
        DillPlugin,
        EnumPlugin,
        EventletPlugin,
        GeventPlugin,
        GlfwPlugin,
        ImplicitImports,
        MultiprocessingPlugin,
        NumpyPlugin,
        PbrPlugin,
        PkgResourcesPlugin,
        PmwPlugin,
        PySidePyQtPlugin,
        TensorflowPlugin,
        TkinterPlugin,
        TorchPlugin,
        ZmqPlugin,
    )

    for plugin_module in [
        AntiBloat,
        ConsiderPyLintAnnotationsPlugin,
        DataFileCollectorPlugin,
        DillPlugin,
        EnumPlugin,
        EventletPlugin,
        GeventPlugin,
        GlfwPlugin,
        ImplicitImports,
        MultiprocessingPlugin,
        NumpyPlugin,
        PbrPlugin,
        PkgResourcesPlugin,
        PmwPlugin,
        PySidePyQtPlugin,
        TensorflowPlugin,
        TkinterPlugin,
        TorchPlugin,
        ZmqPlugin,
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


def setup():
    disableStaticallyLinkedPython()
    useEngagedCommands()
    useEngagedEnvironment()
    loadPlugins()
    os.environ.update(
        {
            "PYTHONPATH": os.pathsep.join(sys.path),
        }
    )
    variants.enable("python")


def main():
    from nuitka import (
        MainControl,
        Options,
    )  # MUST load after all the hack have been set

    Options.parseArgs(will_reexec=False)
    Options.commentArgs()
    MainControl.main()
