import unittest


class TestStringMethods(unittest.TestCase):

    def test_vswhere(self):
        import vswhere
        print(vswhere.get_vswhere_path())
        print(vswhere.find(products='*'))
        print(vswhere.get_latest(products='*'))
        print(vswhere.get_latest_version(products='*'))


if __name__ == '__main__':
    unittest.main()
