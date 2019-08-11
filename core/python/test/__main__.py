import unittest
import click


@click.command()
@click.pass_context
def test():
    loader = unittest.TestLoader()
    suite = loader.discover('.')
    runner = unittest.TextTestRunner()
    runner.run(suite)


test(auto_envvar_prefix='KF_TEST')
