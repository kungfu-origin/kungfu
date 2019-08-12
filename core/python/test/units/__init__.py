import unittest
import click
from test import test


@test.command()
def units():
    loader = unittest.TestLoader()
    suite = loader.discover('.')
    runner = unittest.TextTestRunner()
    runner.run(suite)
