from . import node
from . import python

runners = {
    "node": node.main,
    "python": python.main,
}
