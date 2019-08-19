import pyyjj
import codecs
import click
from kungfu.command import kfc, pass_ctx_from_parent as pass_ctx_from_root
from extensions import EXTENSION_REGISTRY_MD, EXTENSION_REGISTRY_TD, ACCOUNT_SCHEMA
from kungfu.yijinjing.log import create_logger
from kungfu.data.sqlite.data_proxy import AccountsDB


@kfc.group(help_priority=2)
@click.option('-s', '--source', required=True, type=click.Choice(EXTENSION_REGISTRY_MD.names()), help='account source')
@click.pass_context
def account(ctx, source):
    pass_ctx_from_root(ctx)
    ctx.location = pyyjj.location(pyyjj.mode.LIVE, pyyjj.category.SYSTEM, 'etc', 'kungfu', ctx.locator)
    ctx.db = AccountsDB(ctx.location, 'accounts')
    ctx.schema = ACCOUNT_SCHEMA[source]


def pass_ctx_from_parent(ctx):
    pass_ctx_from_root(ctx)
    ctx.location = ctx.parent.location
    ctx.db = ctx.parent.db
    ctx.schema = ctx.parent.schema


def check(value, config):
    if config['type'] == 'int':
        return value.isdigit()
    return True


def make_questions(schema):
    def make_question(config):
        question = {
            'type': 'input' if config['type'] != 'password' else 'password',
            'name': config['key'],
            'message': config['errMsg'],
            'validate': lambda value: check(value, config)
        }
        return question
    return map(make_question, schema['config'])


def encrypt(answers):
    if 'password' in answers:
        answers['password'] = codecs.encode(answers['password'], 'rot13')
    return answers
