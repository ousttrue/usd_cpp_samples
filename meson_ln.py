import pathlib
import os

HERE = pathlib.Path(__file__).parent


def process(subprojects: pathlib.Path):
    print(subprojects)
    src = subprojects / "packagefiles/usd"
    dst = subprojects / "usd"

    for f in src.glob("**/meson.build"):
        rel = f.relative_to(src)
        target = dst / rel
        print(f"remove {target}")
        os.unlink(target)
        print(f"{f}")
        print(f"  => {target}")
        os.symlink(f, target)


if __name__ == "__main__":
    process(HERE / "subprojects")
