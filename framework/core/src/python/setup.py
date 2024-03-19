#  SPDX-License-Identifier: Apache-2.0

import json

from os import path
from pathlib import Path
from poetry.core.factory import Factory
from poetry.core.masonry.builders.sdist import SdistBuilder
from setuptools import find_packages
from setuptools import setup
from setuptools.dist import Distribution

package = Factory().create_poetry(Path(".").resolve()).package
dependencies, extras = SdistBuilder.convert_dependencies(package, package.requires)

install_requires = []
for r in package.requires:
    install_requires.append(f"{r.name}>={r.constraint.flatten()[0].min}")

build_info_path = path.join(path.dirname(__file__), "kungfubuildinfo.json")
with open(build_info_path, "r") as build_info_file:
    build_info = json.load(build_info_file)


class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""

    def has_ext_modules(self):
        return True


setup(
    name=package.name,
    version=build_info["version"],
    license=package.license.id,
    author=package.authors[0],
    url=package.homepage,
    project_urls={"repository": package.repository_url},
    packages=[""] + find_packages(exclude=["test"]),
    package_data={"": ["*.dll", "*.dylib", "*.pyd", "*.so", "*.so.*", "*.json"]},
    include_package_data=True,
    install_requires=install_requires,
    entry_points={"console_scripts": ["kfc = kungfu.__main__:main"]},
    distclass=BinaryDistribution,
)
