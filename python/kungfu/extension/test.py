import extensions
import kungfu.extension as kfext

@kfext.command(help='test ext')
def test(args, logger):
    print('hello kungfu')