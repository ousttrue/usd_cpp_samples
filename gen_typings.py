import pathlib
import os
import io
import sys
import types
import importlib

HERE = pathlib.Path(__file__).absolute().parent

sys.path.insert(0, str(HERE / 'build_release/lib/python'))
os.environ['PATH'] = f'{HERE / "build_release/bin"};{HERE / "build_release/lib"};{os.environ["PATH"]}'


def generate_method(w: io.IOBase, key: str, value):
    w.write(f'    def {key}(*args):')
    doc = getattr(value, '__doc__')
    if doc:
        indent = "        "
        w.write(f'\n{indent}"""\n')
        for l in doc.splitlines():
            w.write(f'{indent}{l}\n')
        w.write(f'{indent}"""\n')
        w.write(f'{indent}...\n')
    else:
        w.write(' ...\n')


def generate_type(w: io.IOBase, key: str, value: type):
    w.write(f'class {key}:\n')
    doc = getattr(value, '__doc__')
    if doc:
        w.write('    """\n')
        for l in doc.splitlines():
            w.write(f'    {l}\n')
        w.write('    """\n')
    for kk in dir(value):
        if kk in ('__class__', '__dict__', '__dir__', '__doc__', '__format__', '__module__', '__reduce_ex__', '__sizeof__', '__weakref__', '__instance_size__'):
            continue
        vv = getattr(value, kk)
        match vv:
            case types.BuiltinMethodType():
                generate_method(w, kk, vv)
            case types.BuiltinFunctionType():
                pass
            case types.WrapperDescriptorType():
                pass
            case types.FunctionType():
                w.write(f'    def {kk}(*args): ...\n')
            case str():
                pass
            case property():
                pass
            case _:
                print(kk, type(vv))
    w.write(f'    pass\n')


def generate_typings(m: types.ModuleType, dst: pathlib.Path):
    dst.parent.mkdir(parents=True, exist_ok=True)
    with dst.open('w', encoding='utf-8') as w:
        for key in dir(m):
            if key.startswith('__'):
                continue
            value = getattr(m, key)
            match value:
                case type():
                    # print('type', key)
                    generate_type(w, key, value)
                case _:
                    print(key, type(value))


if __name__ == '__main__':
    path = pathlib.Path(os.environ['USERPROFILE']) / \
        "Desktop/Kitchen_set/Kitchen_set.usd"
    assert path.exists()

    # import pxr
    # generate_typings(pxr, HERE / 'typings')

    pxr_dir = HERE / 'build_release/lib/python/pxr'
    for f in pxr_dir.iterdir():
        if f.is_dir():
            m = importlib.import_module(f'pxr.{f.stem}')
            generate_typings(m, HERE / f'typings/pxr/{f.stem}/__init__.py')
