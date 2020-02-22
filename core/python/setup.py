from setuptools import find_packages
from setuptools import setup
import json

with open('build_info.json', 'r') as build_info_file:
    build_info = json.load(build_info_file)

setup(
    name="kungfu",
    version=build_info['version'],
    author="taurus.ai",
    license="Apache-2.0",
    packages=[''] + find_packages(exclude=["hooks", "test", "extensions"]),
    package_data={
        '': ['*.so', '*.dylib', '*.dll', '*.json']
    },
    include_package_data=True,
    install_requires=[
        "click>=5.1",
        "sqlalchemy==1.3.8",
        "psutil==5.6.2",
        "numpy",
        "pandas",
        "tabulate==0.8.3",
        "PyInquirer==1.0.3",
        "prompt_toolkit==1.0.14",
        "rx==3.0.1"
    ],
    entry_points={"console_scripts": ["kfc = kungfu.__main__:main"]},
)
