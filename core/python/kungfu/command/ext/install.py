import re
import os, shutil
import tarfile
import extensions
import click
from kungfu.command.ext import ext as kfext, pass_ctx_from_parent
from runpy import run_path
from kungfu import __version__ as kf_version

@kfext.command(help='install extension')
@click.option("-f", "--file", required=True, help="KungFu extension file (tgz)")
@click.pass_context
def install(ctx, file):
    pass_ctx_from_parent(ctx)
    filename = os.path.basename(file)
    filename_re = re.match(r'kfext_(\w+)-v\d+\.\d+\.\d+\.tgz', filename)
    if filename_re:
        ext_name = filename_re.group(1)
        install_path = os.path.join(extensions.extension_path, ext_name)
        ext_cache_dir = os.path.join(extensions.extension_path, '.cache')
        os.makedirs(ext_cache_dir, exist_ok=True)
        extfile = tarfile.open(file)
        if not extfile.getmember('package'):
            click.echo('Bad extension file')
            ctx.logger.error('Bad extension file')
        else:
            ctx.logger.info('extracting extension file %s to %s', file, ext_cache_dir)
            extfile.extractall(ext_cache_dir)
            config_file = os.path.join(ext_cache_dir, "package", "_config.py")
            if os.path.exists(config_file):
                required_sdk_version = run_path(config_file)["KF_SDK_VERSION"]
                if not kf_version == required_sdk_version:
                    ctx.logger.error("KF_SDK_VERSION {} required but {} provided".format(required_sdk_version, kf_version))
                    shutil.rmtree(ext_cache_dir)
                    raise click.Abort()
            else:
                ctx.logger.warn("no required KF_SDK_VERSION info found")
            if os.path.exists(install_path):
                ctx.logger.info('Overwriting extension %s', ext_name)
                shutil.rmtree(install_path)
            os.rename(os.path.join(ext_cache_dir, 'package'), install_path)
            ctx.logger.info("extension %s installed", ext_name)
            click.echo('Installed extension %s' % ext_name)
    else:
        print('Bad extension filename', filename)
        ctx.logger.error("Invalid extension filename %s", file)