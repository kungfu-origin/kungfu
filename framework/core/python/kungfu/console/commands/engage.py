import click
import json
import kungfu
import pkgutil
import os
import sys
import subprocess

from os.path import abspath, basename, dirname

from kungfu.console.commands import (
    CLI,
    PrioritizedCommandGroup,
    get_current_context,
    kfc,
    update_wrapper,
    typing,
)
from kungfu.console.variants import enable as enable_kfc_variant
from kungfu.yijinjing.log import create_logger


def engage_command_context():
    def reset_sys_argv(f: CLI) -> CLI:
        def new_func(*args, **kwargs):
            copy_from_parent = kfc.pass_context("logger")
            sys.argv = [sys.argv[0], *get_current_context().args]
            return copy_from_parent(f)(*args, **kwargs)

        return update_wrapper(typing.cast(CLI, new_func), f)

    return reset_sys_argv


@kfc.group(cls=PrioritizedCommandGroup, help_priority=-1)
@click.help_option("-h", "--help")
@kfc.pass_context()
def engage(ctx):
    ctx.logger = create_logger("engage", ctx.log_level, ctx.console_location)


@engage.command(
    help="Format python files with [Black](https://github.com/psf/black)",
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@engage_command_context()
def black(ctx):
    from black.__main__ import patched_main as main

    main()


@engage.command(
    help="Manage python packages with [pdm](https://pdm.fming.dev)",
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@engage_command_context()
def pdm(ctx):
    from pdm.core import Core
    from pdm.cli.commands.base import BaseCommand
    from pdm.formats.base import array_of_inline_tables, make_inline_table
    from pdm.models.requirements import parse_requirement
    from pdm.project.core import atoml, Project

    class MakeupCommand(BaseCommand):
        """Make up pyproject.toml according to settings in package.json"""

        @staticmethod
        def pdm_config_table(pdm_project, name):
            pdm_config = pdm_project.pyproject
            result = pdm_config[name] = pdm_config.get(name, atoml.table())
            # Fix extra trailing newline
            body = result.value.body
            body and body[-1][0] is None and body.pop()
            return result

        def add_arguments(self, parser) -> None:
            parser.add_argument(
                "--encoding",
                default="utf-8",
                help="Encoding of package.json, defaults to utf-8",
            )
            parser.set_defaults(search_parent=False)

        def handle(self, project, options):
            with open("package.json", mode="r", encoding=options.encoding) as file:
                package_json = json.load(file)

            author = package_json["author"]
            build_config = package_json.get("kungfuBuild", {})
            python_config = build_config.get("python", {})
            dependencies = python_config.get("dependencies", {})

            pdm_project = Project(Core(), ".")
            # PDM_PYTHON is not working perfectly, have to manipulate .pdm.toml
            # to enforce using kfc as interpreter.
            pdm_project.project_config["python.path"] = sys.executable
            pdm_project.pyproject_file.touch()

            project = self.pdm_config_table(pdm_project, "project")
            project["name"] = package_json.get("name", "").split("/").pop()
            project["version"] = make_inline_table({"use_scm": True})
            project["license"] = make_inline_table(
                {"text": package_json.get("license", "")}
            )
            project["authors"] = array_of_inline_tables(
                [
                    {
                        "name": author["name"] if isinstance(author, dict) else author,
                        "email": author["email"] if isinstance(author, dict) else "",
                    }
                ]
            )
            project["dynamic"] = ["classifiers", "version"]
            project[
                "requires-python"
            ] = f"~={sys.version_info.major}.{sys.version_info.minor}"
            project["dependencies"] = project.get("dependencies", [])

            build_system = self.pdm_config_table(pdm_project, "build-system")
            build_system["requires"] = ["pdm-pep517"]
            build_system["build-backend"] = "pdm.pep517.api"

            dependencies and pdm_project.add_dependencies(
                requirements={
                    k: parse_requirement(k + v) for k, v in dependencies.items()
                },
                show_message=False,
            )
            pdm_project.write_pyproject()

    class EngagedCore(Core):
        def __init__(self):
            super().__init__()

        def init_parser(self):
            super().init_parser()
            try:
                from pdm.cli import commands

                next(pkgutil.iter_modules(commands.__path__))
            except StopIteration:
                self.register_pdm_commands()
            self.register_command(MakeupCommand, "makeup")

        def register_pdm_commands(self):
            from pdm.cli.commands import (
                add,
                build,
                cache,
                completion,
                config,
                export,
                import_cmd,
                info,
                init,
                install,
                list,
                lock,
                plugin,
                remove,
                run,
                search,
                show,
                sync,
                update,
                use,
            )

            for module in [
                add,
                build,
                cache,
                completion,
                config,
                export,
                import_cmd,
                info,
                init,
                install,
                list,
                lock,
                plugin,
                remove,
                run,
                search,
                show,
                sync,
                update,
                use,
            ]:
                try:
                    cmd = module.Command
                except AttributeError:
                    continue
                name = cmd.name or module.__name__.split(".").pop()
                self.register_command(cmd, name)

    enable_kfc_variant("python")
    os.environ.update(
        {
            "PDM_AUTO_GLOBAL": "1",
            "PDM_USE_VENV": "0",
        }
    )
    EngagedCore().main()


def engaged_nuitka_context(enable_plugins=True):
    def nuitka_context(f: CLI) -> CLI:
        def new_func(*args, **kwargs):
            reset_sys_argv = engage_command_context()
            EngagedNuitka.disableStaticallyLinkedPython()
            EngagedNuitka.useEngagedCommands()
            EngagedNuitka.useEngagedEnvironment()
            enable_plugins and EngagedNuitka.loadPlugins()
            os.environ["PYTHONPATH"] = os.pathsep.join(sys.path)
            return reset_sys_argv(f)(*args, **kwargs)

        return update_wrapper(typing.cast(CLI, new_func), f)

    return nuitka_context


@engage.command(
    help="Compile and bundle python files with [Nuitka](https://nuitka.net)",
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
)
@engaged_nuitka_context()
def nuitka(ctx):
    from nuitka import (
        MainControl,
        Options,
    )  # MUST load after all the hack have been set

    Options.parseArgs(will_reexec=False)
    Options.commentArgs()
    MainControl.main()


@engage.command(
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
    help_priority=-1,
)
@engaged_nuitka_context(enable_plugins=False)
def nuitka_data_composer(ctx):
    from nuitka.tools.data_composer import DataComposer

    DataComposer.main()


@engage.command(
    context_settings=dict(
        ignore_unknown_options=True,
        allow_extra_args=True,
    ),
    help_priority=-1,
)
@engaged_nuitka_context(enable_plugins=False)
def nuitka_scons(ctx):
    from SCons import Script

    Script.main()


class EngagedNuitka:
    @staticmethod
    def disableStaticallyLinkedPython():
        from nuitka import PythonVersions

        PythonVersions.isStaticallyLinkedPython = lambda: False

    @staticmethod
    def useEngagedCommands():
        from nuitka.build import DataComposerInterface, SconsInterface

        def runEngagedDataComposer(source_dir):
            data_composer_opts = (
                [] if basename(sys.executable).startswith("kfc") else ["-m", "kungfu"]
            )
            mapping = {
                "NUITKA_PACKAGE_HOME": dirname(
                    abspath(sys.modules["nuitka"].__path__[0])
                ),
                "PATH": os.environ["PATH"],
                "PYTHONPATH": os.pathsep.join(sys.path),
            }
            blob_filename = DataComposerInterface.getConstantBlobFilename(source_dir)
            with DataComposerInterface.withEnvironmentVarsOverriden(mapping):
                subprocess.check_call(
                    [
                        sys.executable,
                        *data_composer_opts,
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
                + dirname(kungfu.__bindings__.__file__)
            )
            scons_opts = (
                [] if basename(sys.executable).startswith("kfc") else ["-m", "kungfu"]
            )
            return [sys.executable, *scons_opts, "engage", "nuitka-scons"]

        DataComposerInterface.runDataComposer = runEngagedDataComposer
        SconsInterface._getSconsBinaryCall = getEngagedSconsBinaryCall

    @staticmethod
    def useEngagedEnvironment():
        from nuitka.build import SconsUtils

        def withKungfuLib():
            origin = SconsUtils.createEnvironment

            def createEnvironment(**kwargs):
                env = origin(**kwargs)
                env.Append(LIBPATH=dirname(kungfu.__bindings__.__file__))
                return env

            return createEnvironment

        SconsUtils.createEnvironment = withKungfuLib()

    @staticmethod
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
