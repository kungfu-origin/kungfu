from setuptools import find_packages
from setuptools import setup
import json

with open("kungfubuildinfo.json", "r") as build_info_file:
    build_info = json.load(build_info_file)

setup(
    name="kungfu",
    version=build_info["version"],
    author="Kungfu Origin",
    license="Apache-2.0",
    packages=[""] + find_packages(exclude=["test"]),
    package_data={"": ["*.so", "*.dylib", "*.dll", "*.pyd", "*.json"]},
    include_package_data=True,
    install_requires=[
        "click~=8.0.1",
        "psutil~=5.8.0",
        "sortedcontainers~=2.1.0",
        "recordclass~=0.14.3",
        "dotted_dict~=1.1.2",
        "tabulate~=0.8.6",
        "prompt_toolkit~=1.0.14",
        "PyInquirer~=1.0.3",
        "numpy~=1.21.0",
        "pandas~=1.2.0",
        "statsmodels~=0.12.2",
        "plotly~=5.1.0",
        "chinesecalendar~=1.5.1",
    ],
    entry_points={"console_scripts": ["kfc = kungfu.__main__:main"]},
    has_ext_modules=lambda: True,
)
