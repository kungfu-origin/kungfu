
import click
from kungfu.command import kfc, pass_ctx_from_parent, replay_setup

@kfc.command()
@click.pass_context
def unittest(ctx):
    pass_ctx_from_parent(ctx)
    import unittest
    loader = unittest.TestLoader()
    suite = loader.discover("python/kungfu/tests")
    runner = unittest.TextTestRunner()
    runner.run(suite)

