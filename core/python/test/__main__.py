import unittest
import click
import extensions


@click.command()
def test():
    loader = unittest.TestLoader()
    suite = loader.discover('.')
    runner = unittest.TextTestRunner()
    runner.run(suite)


test()
