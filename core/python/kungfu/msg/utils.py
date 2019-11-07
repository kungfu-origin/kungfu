
import re
import inspect

def object_as_dict(obj):
    d = {}
    for attr in dir(obj):
        if not attr.startswith('__'):
            try:
                value = getattr(obj, attr)
                if not inspect.ismethod(value):
                    d[attr] = value
            except UnicodeDecodeError:
                continue
    return d

def flatten_json(y):
    out = {}
    def flatten(x, name=''):
        if type(x) is dict:
            for a in x:
                flatten(x[a], name + a + '_')
        elif type(x) is list:
            i = 0
            for a in x:
                flatten(a, name + str(i) + '_')
                i += 1
        else:
            out[name[:-1]] = x
    flatten(y)
    return out

def underscore(word):
    word = re.sub(r"([A-Z]+)([A-Z][a-z])", r'\1_\2', word)
    word = re.sub(r"([a-z\d])([A-Z])", r'\1_\2', word)
    word = word.replace("-", "_")
    return word.lower()
