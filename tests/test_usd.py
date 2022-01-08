import unittest
import pathlib
import sys
import os
HERE = pathlib.Path(__file__).absolute().parent
BUILD_DIR = HERE.parent / 'build_release'
PY_USD_DIR = BUILD_DIR / 'lib/python'
BIN_DIR0 = BUILD_DIR / 'bin'
BIN_DIR1 = BUILD_DIR / 'lib'

os.environ['PATH'] = f'{BIN_DIR0};{BIN_DIR1};{os.environ["PATH"]}'
sys.path.append(str(PY_USD_DIR))


class TestUsd(unittest.TestCase):

    def test_usd(self):
        from pxr import Usd
        print(Usd)


if __name__ == '__main__':
    unittest.main()
