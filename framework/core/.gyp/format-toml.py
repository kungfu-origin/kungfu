import tomli
import tomli_w
import sys


def format_toml(filename):
    with open(filename, "rb") as target_toml_file:
        target_toml = tomli.load(target_toml_file)

    with open(filename, "wb") as target_toml_file:
        tomli_w.dump(target_toml, target_toml_file)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise IndexError("Expected one argument: filename")
    format_toml(sys.argv[1])
