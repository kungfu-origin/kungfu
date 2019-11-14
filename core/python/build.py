import os
import sys
import subprocess
import platform
import click
from kungfu.version import get_version


@click.group(invoke_without_command=True)
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.option('--build_type', type=click.Choice(['Release', 'Debug']), default='Release', help='build type')
@click.option('--arch', type=click.Choice(['x64', 'x86']), default='x64', help='arch')
@click.option('--runtime', type=click.Choice(['electron', 'node']), default='electron', help='Node.js runtime')
@click.option('--node_version', type=str, default='10.0.0', help='Node.js runtime version')
@click.option('--electron_version', type=str, default='5.0.0', help='Electron runtime version')
@click.pass_context
def build(ctx, log_level, build_type, arch, runtime, node_version, electron_version):
    ctx.log_level = log_level
    ctx.build_type = build_type
    ctx.arch = arch
    ctx.runtime = runtime
    ctx.runtime_version = node_version if runtime == 'node' else electron_version

    if ctx.invoked_subcommand is None:
        click.echo(build.get_help(ctx))
    pass


@build.command()
@click.pass_context
def configure(ctx):
    cmake_configure = build_cmake_js_cmd(ctx, 'configure')
    sys.exit(subprocess.Popen(cmake_configure).wait())


@build.command()
@click.pass_context
def make(ctx):
    cmake_build = build_cmake_js_cmd(ctx, 'build')
    sys.exit(subprocess.Popen(cmake_build).wait())


@build.command()
@click.pass_context
def freeze(ctx):
    os.environ['CMAKE_BUILD_TYPE'] = ctx.parent.build_type

    with open(os.path.join(os.getcwd(), "build", ctx.parent.build_type, "version.info"), 'w') as version_file:
        version_file.write(f"{get_version()}")

    osname = platform.system()

    if osname == 'Linux':
        sys.exit(subprocess.Popen(['pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-unix.spec']).wait())
    if osname == 'Darwin':
        rc = subprocess.Popen(['pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-unix.spec']).wait()
        os.chdir('build/kfc')
        if os.path.exists('.Python'):
            os.rename('.Python', 'Python')
            os.symlink('Python', '.Python')
        sys.exit(rc)
    if osname == 'Windows':
        sys.exit(subprocess.Popen(['pyinstaller', '--clean', '-y', r'--distpath=build', r'python\kfc-win.spec']).wait())


def find(tool):
    tool_path = tool
    if platform.system() == 'Windows':
        for line in subprocess.Popen(['where', tool], stdout=subprocess.PIPE).stdout.readlines():
            path = line.decode('utf8').strip()
            if path.endswith('.cmd'):
                tool_path = path
    return tool_path


def build_cmake_js_cmd(ctx, cmd):
    python_path = subprocess.Popen(["pipenv", "--py"], stdout=subprocess.PIPE).stdout.read().decode().strip()

    spdlog_levels = {
        'trace':        'SPDLOG_LEVEL_TRACE',
        'debug':        'SPDLOG_LEVEL_DEBUG',
        'info':         'SPDLOG_LEVEL_INFO',
        'warning':      'SPDLOG_LEVEL_WARN',
        'error':        'SPDLOG_LEVEL_ERROR',
        'critical':     'SPDLOG_LEVEL_CRITICAL'
    }
    loglevel = spdlog_levels[ctx.parent.log_level]

    cmake_js_cmd = [find('yarn'), 'cmake-js', '--debug' if ctx.parent.build_type == 'Debug' else '',
                    '--arch', ctx.parent.arch,
                    '--runtime', ctx.parent.runtime,
                    '--runtime-version', ctx.parent.runtime_version,
                    '--CDPYTHON_EXECUTABLE=' + python_path,
                    '--CDSPDLOG_LOG_LEVEL_COMPILE=' + loglevel]

    if platform.system() == 'Windows':
        return cmake_js_cmd + ['--toolset', 'host=' + ctx.parent.arch,
                               '--CDCMAKE_GENERATOR_PLATFORM=' + ctx.parent.arch, cmd]
    else:
        return cmake_js_cmd + [cmd]


build(auto_envvar_prefix='KF_BUILD')
