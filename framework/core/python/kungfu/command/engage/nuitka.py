import click
import os
import sys
import subprocess

from os.path import abspath, basename, dirname
from kungfu.command.engage import engage, pass_ctx_from_parent


@engage.command(
    help="Compile and bundle python files with [Nuitka](https://nuitka.net)",
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@click.pass_context
def nuitka(ctx):
    pass_ctx_from_parent(ctx)

    os.environ["PYTHONPATH"] = os.pathsep.join(sys.path)

    sys.argv = [sys.argv[0], *ctx.args]

    def runDataComposer(source_dir):
        data_composer_opts = (
            [] if basename(sys.executable).startswith("kfc") else ["-m", "kungfu"]
        )
        mapping = {
            "NUITKA_PACKAGE_HOME": dirname(abspath(sys.modules["nuitka"].__path__[0])),
            "PATH": os.environ["PATH"],
            "PYTHONPATH": os.pathsep.join(sys.path),
        }

        from nuitka.build import DataComposerInterface

        blob_filename = DataComposerInterface.getConstantBlobFilename(source_dir)

        with DataComposerInterface.withEnvironmentVarsOverriden(mapping):
            subprocess.check_call(
                [
                    sys.executable,
                    *data_composer_opts,
                    "engage",
                    "data-composer",
                    source_dir,
                    blob_filename,
                ],
                shell=False,
            )

        return blob_filename

    def getSconsBinaryCall():
        import kungfu

        os.environ["PYTHONPATH"] = (
            dirname(dirname(kungfu.__file__))
            + os.pathsep
            + dirname(kungfu.__bindings__.__file__)
        )
        scons_opts = (
            [] if basename(sys.executable).startswith("kfc") else ["-m", "kungfu"]
        )
        return [sys.executable, *scons_opts, "engage", "scons"]

    from nuitka import PythonVersions

    PythonVersions.isStaticallyLinkedPython = lambda: False

    from nuitka.build import DataComposerInterface

    DataComposerInterface.runDataComposer = runDataComposer

    from nuitka.build import SconsInterface

    SconsInterface._getSconsBinaryCall = getSconsBinaryCall

    from nuitka.plugins import Plugins
    from nuitka.plugins.standard import AntiBloat
    from nuitka.plugins.standard import ConsiderPyLintAnnotationsPlugin
    from nuitka.plugins.standard import DataFileCollectorPlugin
    from nuitka.plugins.standard import DillPlugin
    from nuitka.plugins.standard import EnumPlugin
    from nuitka.plugins.standard import EventletPlugin
    from nuitka.plugins.standard import GeventPlugin
    from nuitka.plugins.standard import GlfwPlugin
    from nuitka.plugins.standard import ImplicitImports
    from nuitka.plugins.standard import MultiprocessingPlugin
    from nuitka.plugins.standard import NumpyPlugin
    from nuitka.plugins.standard import PbrPlugin
    from nuitka.plugins.standard import PkgResourcesPlugin
    from nuitka.plugins.standard import PmwPlugin
    from nuitka.plugins.standard import PySidePyQtPlugin
    from nuitka.plugins.standard import TensorflowPlugin
    from nuitka.plugins.standard import TkinterPlugin
    from nuitka.plugins.standard import TorchPlugin
    from nuitka.plugins.standard import ZmqPlugin

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

    from nuitka import Options

    Options.parseArgs(will_reexec=False)
    Options.commentArgs()

    from nuitka import MainControl

    MainControl.main()


@engage.command(
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@click.pass_context
def data_composer(ctx):
    pass_ctx_from_parent(ctx)

    sys.argv = [sys.argv[0], *ctx.args]

    from nuitka import PythonVersions

    PythonVersions.isStaticallyLinkedPython = lambda: False

    from nuitka.tools.data_composer.DataComposer import main

    main()


@engage.command(
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@click.pass_context
def scons(ctx):
    pass_ctx_from_parent(ctx)

    sys.argv = [sys.argv[0], *ctx.args]

    from nuitka import Options

    sys.path.append(
        os.path.join(
            os.path.dirname(Options.__file__),
            "build",
            "inline_copy",
            "lib",
            "scons-3.1.2",
        )
    )

    from nuitka import PythonVersions

    PythonVersions.isStaticallyLinkedPython = lambda: False

    from nuitka.build import SconsUtils

    originCreateEnvironment = SconsUtils.createEnvironment

    def createEnvironment(**kwargs):
        import kungfu

        env = originCreateEnvironment(**kwargs)
        env.Append(LIBPATH=dirname(kungfu.__bindings__.__file__))
        return env

    SconsUtils.createEnvironment = createEnvironment

    from SCons.Script import main

    main()
