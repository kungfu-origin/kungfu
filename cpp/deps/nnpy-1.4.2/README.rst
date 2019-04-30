nnpy: cffi-based Python bindings for nanomsg
============================================

Is what it says on the tin. Stay tuned for more. Dependencies:

- Python 2.7 or 3.4
- cffi (http://cffi.readthedocs.org/en/latest/) and its dependencies
- nanomsg (tested with 1.0)

How to install
--------------

The usual should work:

.. code-block:: shell

   $ sudo pip install nnpy

Getting started
---------------

.. code-block:: python
   
   import nnpy
   
   pub = nnpy.Socket(nnpy.AF_SP, nnpy.PUB)
   pub.bind('inproc://foo')
   
   sub = nnpy.Socket(nnpy.AF_SP, nnpy.SUB)
   sub.connect('inproc://foo')
   sub.setsockopt(nnpy.SUB, nnpy.SUB_SUBSCRIBE, '')
   
   pub.send('hello, world')
   print(sub.recv())

Related Projects
----------------

- `nanomsg <http://nanomsg.org/>`_
- `aionn <https://github.com/wrobell/aionn>`_ - asyncio messaging library based on nnpy
