import pathlib
import os

HERE = pathlib.Path(__file__).parent


def process(subprojects: pathlib.Path):
    print(subprojects)
    src = subprojects / "packagefiles"
    dst = subprojects

    for f in src.glob("**/meson.build"):
        rel = f.relative_to(src)
        target = dst / rel
        if target.exists() or target.is_symlink():
            print(f"remove {target}")
            os.unlink(target)
        if target.parent.exists():
            print(f"{f}")
            print(f"  => {target}")
            os.symlink(f, target)


if __name__ == "__main__":
    process(HERE / "subprojects")
