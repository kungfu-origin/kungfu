from cffi import FFI
import os

try:
    import ConfigParser as cfgparser
except ImportError:
    # try py3 import
    import configparser as cfgparser

SITE_CFG = 'site.cfg'

DEFAULT_INCLUDE_DIRS = ['../nanomsg-1.1.5/include/nanomsg']
DEFAULT_HOST_LIBRARY = 'nanomsg'
DEFAULT_LIBRARY_DIR = '../../../' + os.environ['CMAKEBUILDTYPE']

import platform
if platform.system()=="Darwin":
    DEFAULT_HOST_LIBRARY = r'../../../' + os.environ['CMAKEBUILDTYPE'] + '/libnanomsg.dylib'
if platform.system()=="Linux":
    DEFAULT_HOST_LIBRARY = r'../../../' + os.environ['CMAKEBUILDTYPE'] + '/libnanomsg.so'
if platform.system()=="Windows":
    DEFAULT_LIBRARY_DIR = '..\\nanomsg-1.1.5\\src\\' + os.environ['CMAKEBUILDTYPE']
    DEFAULT_HOST_LIBRARY = '..\\..\\..\\' + os.environ['CMAKEBUILDTYPE'] + '\\nanomsg.dll'

BLOCKS = {'{': '}', '(': ')'}
DEFINITIONS = '''
struct nn_pollfd {
    int fd;
    short events;
    short revents;
};
'''

def header_files(include_paths):
    for dir in include_paths:
        if os.path.exists(dir):
            break
    return {fn: os.path.join(dir, fn) for fn in os.listdir(dir)
            if fn.endswith('.h')}

def functions(hfiles):

    lines = []
    for fn, path in hfiles.items():
        with open(path) as f:
            cont = ''
            for ln in f:

                if cont == ',':
                    lines.append(ln)
                    cont = ''
                if cont in BLOCKS:
                    lines.append(ln)
                    if BLOCKS[cont] in ln:
                        cont = ''
                if not (ln.startswith('NN_EXPORT')
                    or ln.startswith('typedef')):
                    continue

                lines.append(ln)
                cont = ln.strip()[-1]

    return ''.join(ln[10:] if ln.startswith('NN_') else ln for ln in lines)

def symbols(ffi, host_library):

    nanomsg = ffi.dlopen(host_library)
    lines = []
    for i in range(1024):

        val = ffi.new('int*')
        name = nanomsg.nn_symbol(i, val)
        if name == ffi.NULL:
            break

        name = ffi.string(name).decode()
        name = name[3:] if name.startswith('NN_') else name
        lines.append('%s = %s' % (name, val[0]))

    return '\n'.join(lines) + '\n'

def create_module():

    # Set defaults

    set_source_args = {
        'include_dirs': DEFAULT_INCLUDE_DIRS
    }
    host_library = DEFAULT_HOST_LIBRARY

    # Read overrides for cross-compilation support from site.cfg

    if os.path.isfile(SITE_CFG):
        parser = cfgparser.ConfigParser()

        if parser.read(SITE_CFG):

            parsed_cfg = parser.defaults()
            for param in ['include_dirs', 'library_dirs']:
                if param in parsed_cfg:
                    set_source_args[param] = parsed_cfg[param].split(',')

            if 'host_library' in parsed_cfg:
                host_library = parsed_cfg['host_library']

    # Add some more directories from the environment

    if 'CPATH' in os.environ:
        cpaths = os.getenv('CPATH').split(os.pathsep)
        set_source_args['include_dirs'] += [os.path.join(p, 'nanomsg')
                                            for p in cpaths]

    hfiles = header_files(set_source_args['include_dirs'])
    # remove ws.h due to https://github.com/nanomsg/nanomsg/issues/467
    hfiles.pop('ws.h', None)

    # Build FFI module and write out the constants

    ffi = FFI()
    ffi.cdef(DEFINITIONS)
    ffi.cdef(functions(hfiles))
    ffi.set_source('_nnpy', '\n'.join('#include <%s>' % fn for fn in hfiles),
                   library_dirs=[DEFAULT_LIBRARY_DIR], libraries=['nanomsg'], **set_source_args)

    with open('nnpy/constants.py', 'w') as f:
        f.write(symbols(ffi, host_library))

    return ffi

ffi = create_module()

if __name__ == '__main__':
    ffi.compile()
