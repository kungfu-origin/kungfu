import json

from os import path
from pathlib import Path
from poetry.core.factory import Factory
from poetry.core.masonry.builders.sdist import SdistBuilder
from setuptools import find_packages
from setuptools import setup

package = Factory().create_poetry(Path(".").resolve()).package
dependencies, extras = SdistBuilder.convert_dependencies(package, package.requires)

build_info_path = path.join(path.dirname(__file__), "kungfubuildinfo.json")
with open(build_info_path, "r") as build_info_file:
    build_info = json.load(build_info_file)

setup(
    name=package.name,
    version=build_info["version"],
    license=package.license.id,
    author=package.authors,
    url=package.homepage,
    project_urls={"repository": package.repository_url},
    packages=[""] + find_packages(exclude=["test"]),
    package_data={"": ["*.so", "*.dylib", "*.dll", "*.pyd", "*.json"]},
    include_package_data=True,
    install_requires=dependencies,
    entry_points={"console_scripts": ["kfc = kungfu.__main__:main"]},
    has_ext_modules=lambda: True,
)
