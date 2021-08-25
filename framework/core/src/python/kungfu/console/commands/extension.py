import click
import kungfu_extensions
import os
import re
import shutil
import tarfile

from kungfu import __version__ as kf_version
from runpy import run_path

from kungfu.console.commands import kfc, PrioritizedCommandGroup
from kungfu.yijinjing.log import create_logger


extension_command_context = kfc.pass_context("logger")


@kfc.group(cls=PrioritizedCommandGroup)
@click.help_option("-h", "--help")
@kfc.pass_context()
def extension(ctx):
    kfc.pass_ctx_from_parent(ctx)
    ctx.logger = create_logger("ext", ctx.log_level, ctx.console_location)
    if not os.getenv("KF_NO_EXT"):
        pass
    else:
        print("Extension disabled by KF_NO_EXT")
        ctx.logger.warning("Trying to manage extension while disallowed by KF_NO_EXT")


def pass_ctx_from_parent(ctx):
    kfc.pass_ctx_from_parent(ctx)
    ctx.logger = ctx.parent.logger


@extension.command(help="install extension")
@click.option("-f", "--file", required=True, help="KungFu extension file (tgz)")
@extension_command_context
def install(ctx, file):
    pass_ctx_from_parent(ctx)
    filename = os.path.basename(file)
    filename_re = re.match(r"kfext_(\w+)-v\d+\.\d+\.\d+\.tgz", filename)
    if filename_re:
        ext_name = filename_re.group(1)
        install_path = os.path.join(kungfu_extensions.extension_path, ext_name)
        ext_cache_dir = os.path.join(kungfu_extensions.extension_path, ".cache")
        os.makedirs(ext_cache_dir, exist_ok=True)
        extfile = tarfile.open(file)
        if not extfile.getmember("package"):
            click.echo("Bad extension file")
            ctx.logger.error("Bad extension file")
        else:
            ctx.logger.info("extracting extension file %s to %s", file, ext_cache_dir)
            extfile.extractall(ext_cache_dir)
            config_file = os.path.join(ext_cache_dir, "package", "_config.py")
            if os.path.exists(config_file):
                required_sdk_version = run_path(config_file)["KF_SDK_VERSION"]
                if not kf_version == required_sdk_version:
                    ctx.logger.error(
                        "KF_SDK_VERSION {} required but {} provided".format(
                            required_sdk_version, kf_version
                        )
                    )
                    shutil.rmtree(ext_cache_dir)
                    raise click.Abort()
            else:
                ctx.logger.warn("no required KF_SDK_VERSION info found")
            if os.path.exists(install_path):
                ctx.logger.info("Overwriting extension %s", ext_name)
                shutil.rmtree(install_path)
            os.rename(os.path.join(ext_cache_dir, "package"), install_path)
            ctx.logger.info("extension %s installed", ext_name)
            click.echo("Installed extension %s" % ext_name)
    else:
        print("Bad extension filename", filename)
        ctx.logger.error("Invalid extension filename %s", file)


@extension.command(help="uninstall extension")
@click.option("-n", "--name", required=True, help="KungFu extension name")
@extension_command_context
def uninstall(ctx, name):
    pass_ctx_from_parent(ctx)
    install_path = os.path.join(kungfu_extensions.extension_path, name)
    if os.path.exists(install_path):
        shutil.rmtree(install_path)
        ctx.parent.logger.info("Uninstalled extension %s", name)
        click.echo("Uninstalled extension " + name)
    else:
        click.echo("Extension " + name + "not found")


@extension.command("list", help="list extensions")
@extension_command_context
def list_cmd(ctx):
    pass_ctx_from_parent(ctx)
    click.echo("Installed extensions:")
    for ext_name in kungfu_extensions.EXTENSIONS.keys():
        ext_funcs = []
        if kungfu_extensions.EXTENSION_REGISTRY_MD.has_extension(ext_name):
            ext_funcs.append("md")
        if kungfu_extensions.EXTENSION_REGISTRY_TD.has_extension(ext_name):
            ext_funcs.append("td")
        click.echo("%s %s" % (ext_name, ext_funcs))
