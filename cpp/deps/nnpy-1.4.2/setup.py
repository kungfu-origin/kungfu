from setuptools import setup
import os, platform, glob, subprocess, shutil

os.chdir(os.path.join('build', 'cpp', 'deps', 'nnpy-1.4.2'))

for _nnpy in glob.glob("build/lib*/_nnpy*"):
    os.remove(_nnpy)

setup(
    name='nnpy',
    version='1.4.2',
    url='https://github.com/nanomsg/nnpy',
    license='MIT',
    author='Dirkjan Ochtman',
    author_email='dirkjan@ochtman.nl',
    description='cffi-based Python bindings for nanomsg',
    long_description=open('README.rst').read(),
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Web Environment',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: Implementation :: CPython',
    ],
    packages=['nnpy'],
    setup_requires=["cffi>=1.0.0"],
    cffi_modules=["generate.py:ffi"],
    install_requires=['cffi'],
    dependency_links=[
        "https://pypi.tuna.tsinghua.edu.cn/simple"
    ]
)

nnpy_lib = glob.glob("build/lib*/_nnpy*")[0]
if platform.system() == 'Darwin':
    install_name_tool = subprocess.Popen(["install_name_tool", "-add_rpath", "@loader_path", nnpy_lib])
    install_name_tool.wait()
    if install_name_tool.returncode != 0:
        raise Exception("failed to augment rpath for nnpy abi3 lib")
    else:
        print("Augmented rpath for nnpy abi3 lib")
shutil.copy(nnpy_lib, os.path.join('..', '..', '..', os.environ['CMAKEBUILDTYPE']))
