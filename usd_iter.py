import pathlib

HERE = pathlib.Path(__file__).absolute().parent


def usd_iter(path: pathlib.Path):
    for f in path.iterdir():
        if f.is_file() and f.suffix == '.lib':
            if f.name.startswith('usd_'):
                print(f.name)


usd_iter(HERE / 'build_usd/lib')
