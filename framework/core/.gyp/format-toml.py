import tomli
import tomli_w


def format_toml(filename):
    with open(filename, "rb") as poetry_lock_file:
        poetry_lock = tomli.load(poetry_lock_file)

    with open(filename, "wb") as poetry_lock_file:
        tomli_w.dump(poetry_lock, poetry_lock_file)


if __name__ == 'main':
    format_toml("poetry.lock")
