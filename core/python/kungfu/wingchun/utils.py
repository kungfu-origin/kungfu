
def to_dict(obj):
    return { attr: getattr(obj) for attr in dir(obj) if not attr.startswith('__')}