import pyyjj
import os
import click
from pip._internal import main as pip_main
from pip._vendor import pkg_resources
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from kungfu.yijinjing.log import create_logger

def find_on_path(path):
    dists = pkg_resources.find_on_path(None, path)
    dists = sorted(dists, key=lambda item: str(item))
    return dists

@kfc.group()
@click.pass_context
def package(ctx):
    pass_ctx_from_root(ctx)
    ctx.location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'util', 'package', ctx.locator)
    ctx.logger = create_logger('package', ctx.log_level, ctx.location)

def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.logger = ctx.parent.logger

@package.command(help='Install python package from local path')
@click.option("-d", "--dir", required=True, help="Find packages from",)
@click.option("-r", "--requirements", required=True,help="Requirements file")
@click.pass_context
def install(ctx, dir, requirements):
    pass_ctx_from_parent(ctx)
    pip_main(['install', "--no-index", "--find-links={}".format(dir), "-t", ctx.package_path, "-r", requirements])

@package.command(help='List python package installed')
@click.pass_context
def list(ctx):
    pass_ctx_from_parent(ctx)
    dists = find_on_path(ctx.package_path)
    print("package Installed at path: {}".format(ctx.package_path))
    for dist in dists:
        print('{}=={}'.format(dist.project_name, dist.version))
