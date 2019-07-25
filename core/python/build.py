import os
import subprocess
import platform
import click


@click.group(invoke_without_command=True)
@click.option('-l', '--log_level', type=click.Choice(['trace', 'debug', 'info', 'warning', 'error', 'critical']),
              default='warning', help='logging level')
@click.option('--build_type', type=click.Choice(['Release', 'Debug']), default='Release', help='build type')
@click.option('--arch', type=click.Choice(['x64', 'ia32']), default='x64', help='arch')
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
    set_cmake_var(ctx)
    cmake_configure = [find('yarn'), 'cmake-js', 'configure', '--debug' if ctx.parent.build_type == 'Debug' else '',
                       '--arch', ctx.parent.arch, '--runtime', ctx.parent.runtime, '--runtime-version', ctx.parent.runtime_version]
    subprocess.Popen(cmake_configure).wait()


@build.command()
@click.pass_context
def make(ctx):
    set_cmake_var(ctx)
    cmake_build = [find('yarn'), 'cmake-js', 'build', '--debug' if ctx.parent.build_type == 'Debug' else '',
                   '--arch', ctx.parent.arch, '--runtime', ctx.parent.runtime, '--runtime-version', ctx.parent.runtime_version]
    subprocess.Popen(cmake_build).wait()


@build.command()
@click.pass_context
def package(ctx):
    os.environ['CMAKE_BUILD_TYPE'] = ctx.parent.build_type

    osname = platform.system()

    if osname == 'Linux':
        subprocess.Popen(['pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-unix.spec']).wait()
    if osname == 'Darwin':
        subprocess.Popen(['pyinstaller', '--clean', '-y', '--distpath=build', 'python/kfc-unix.spec']).wait()
        os.chdir('build/kfc')
        os.rename('.Python', 'Python')
        os.symlink('Python', '.Python')
    if osname == 'Windows':
        subprocess.Popen(['pyinstaller', '--clean', '-y', r'--distpath=build', r'python\kfc-win.spec']).wait()


def find(tool):
    tool_path = tool
    if platform.system() == 'Windows':
        for line in subprocess.Popen(['where', tool], stdout=subprocess.PIPE).stdout.readlines():
            path = line.decode('utf8').strip()
            if path.endswith('.cmd'):
                tool_path = path
    return tool_path


def set_cmake_var(ctx):
    python_path = subprocess.Popen(["pipenv", "--py"], stdout=subprocess.PIPE).stdout.read().decode().strip()
    npm = find('npm')

    spdlog_levels = {
        'trace':        'SPDLOG_LEVEL_TRACE',
        'debug':        'SPDLOG_LEVEL_DEBUG',
        'info':         'SPDLOG_LEVEL_INFO',
        'warning':      'SPDLOG_LEVEL_WARN',
        'error':        'SPDLOG_LEVEL_ERROR',
        'critical':     'SPDLOG_LEVEL_CRITICAL'
    }
    loglevel = spdlog_levels[ctx.parent.log_level]

    cmake_py_exe = [npm, 'config', 'set', 'cmake_PYTHON_EXECUTABLE', python_path]
    click.echo(' '.join(cmake_py_exe))
    subprocess.Popen(cmake_py_exe).wait()

    cmake_spdlog = [npm, 'config', 'set', 'cmake_SPDLOG_LOG_LEVEL_COMPILE', loglevel]
    click.echo(' '.join(cmake_spdlog))
    subprocess.Popen(cmake_spdlog).wait()

    if platform.system() == 'Windows':
        cmake_arch = [npm, 'config', 'set', 'cmake_CMAKE_GENERATOR_PLATFORM', ctx.parent.arch]
        click.echo(' '.join(cmake_arch))
        subprocess.Popen(cmake_arch).wait()


build(auto_envvar_prefix='KF_BUILD')
