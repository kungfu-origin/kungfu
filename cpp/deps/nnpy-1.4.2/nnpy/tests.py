from __future__ import print_function
import nnpy, unittest

class Tests(unittest.TestCase):
    def test_basic(self):

        pub = nnpy.Socket(nnpy.AF_SP, nnpy.PUB)
        pub.setsockopt(nnpy.SOL_SOCKET, nnpy.IPV4ONLY, 0)
        pub.bind('inproc://foo')
        self.assertEqual(pub.getsockopt(nnpy.SOL_SOCKET, nnpy.DOMAIN), 1)

        sub = nnpy.Socket(nnpy.AF_SP, nnpy.SUB)
        sub_conn = sub.connect('inproc://foo')
        sub.setsockopt(nnpy.SUB, nnpy.SUB_SUBSCRIBE, '')

        pub.send('FLUB')
        poller = nnpy.PollSet((sub, nnpy.POLLIN))
        self.assertEqual(len(poller.poll()), 1)
        self.assertEqual(poller.poll()[0], 1)
        self.assertEqual(sub.recv().decode(), 'FLUB')
        self.assertEqual(pub.get_statistic(nnpy.STAT_MESSAGES_SENT), 1)
        pub.close()
        sub.shutdown(sub_conn)

    def test_basic_nn_error(self):
        address = 'inproc://timeout-always'

        req = nnpy.Socket(nnpy.AF_SP, nnpy.REQ)
        req.connect(address)

        req.setsockopt(nnpy.SOL_SOCKET, nnpy.RCVTIMEO, 500)

        with self.assertRaises(nnpy.errors.NNError):
            req.recv()

def suite():
    return unittest.makeSuite(Tests)

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
