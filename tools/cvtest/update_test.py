"""The launcher's own update (launcher.cpp's Updater), as a player's install takes it: a release package installed
outside this repository (TEMP, or /tmp on Linux), marked as an older release in its version.txt, with a player's
files in it (castlevania.toml, a save in hdd, a picture in textures), then
  - CastlevaniaLauncher --update-check: the newest release of the port on GitHub is offered (exit code 3), and
    update/check.txt names it and this system's package (the .zip on Windows, the -linux.tar.xz on Linux)
  - CastlevaniaLauncher --update-apply: it is downloaded, checked against the size and SHA-256 GitHub gives, unpacked
    and put in place (exit code 0): version.txt is its tag, the launcher replaced and the one before kept in
    update/previous, the download and the unpacked folder gone, and the player's files as they were
It downloads the newest release from GitHub (some 78 MB on Windows, 104 MB on Linux).

usage: python -I tools/cvtest/update_test.py <package> [--from <tag>] [--keep]
<package> is release.bat's zip or tools/linux/release.sh's .tar.xz. --from sets the release the install claims to be
(1.0.0 by default: older than any published). The install is deleted after a run that passed, unless --keep.
"""
import hashlib
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
import time
import zipfile
from pathlib import Path

LINUX = sys.platform.startswith("linux")
LAUNCHER = "CastlevaniaLauncher" if LINUX else "CastlevaniaLauncher.exe"


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def unpack(package, into):
    """the package into a new folder; its folder (the one holding the launcher)"""
    if package.name.endswith(".tar.xz"):
        with tarfile.open(package) as tf:
            tf.extractall(into, filter="tar") if hasattr(tarfile, "data_filter") else tf.extractall(into)
    else:
        with zipfile.ZipFile(package) as z:
            z.extractall(into)
    found = [p.parent for p in into.rglob(LAUNCHER)]
    return found[0] if found else None


def run(game, arg, timeout):
    """the launcher without its window: its exit code, and its check.txt's fields"""
    env = dict(os.environ)
    for k in list(env):
        if k.startswith(("XBE_", "CV_")):
            del env[k]
    try:
        code = subprocess.run([str(game / LAUNCHER), arg], cwd=game, env=env, timeout=timeout,
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
    except subprocess.TimeoutExpired:
        return None, []
    check = game / "update" / "check.txt"
    fields = check.read_text(encoding="utf-8", errors="replace").split() if check.exists() else []
    return code, fields


def main():
    args = sys.argv[1:]
    if not args or args[0].startswith("--"):
        print(__doc__)
        return 2
    package = Path(args[0]).resolve()
    old = args[args.index("--from") + 1] if "--from" in args else "1.0.0"
    keep = "--keep" in args
    root = Path(tempfile.gettempdir()).resolve() / f"cvtest-update-{time.strftime('%Y%m%d-%H%M%S')}"
    root.mkdir(parents=True)
    failures = []

    def check(ok, what):
        print(f"  {'ok  ' if ok else 'FAIL'} {what}", flush=True)
        if not ok:
            failures.append(what)
        return ok

    game = unpack(package, root)
    if not check(game is not None, f"{package.name} unpacked into {root}"):
        return 1
    (game / "version.txt").write_text(old + "\n", encoding="utf-8")
    # the player's own files, which an update leaves alone
    mine = {"castlevania.toml": b'texture_pack = true\nupdate_check = true\nlauncher_language = "en"\n',
            "hdd/update-test-save.bin": os.urandom(4096),
            "textures/update-test.png": os.urandom(2048)}
    for rel, data in mine.items():
        (game / rel).parent.mkdir(parents=True, exist_ok=True)
        (game / rel).write_bytes(data)
    before = sha256(game / LAUNCHER)
    before_file = (game / LAUNCHER).stat()  # its file: a release whose launcher did not change has the same bytes

    code, fields = run(game, "--update-check", 180)
    newest = fields[2] if len(fields) > 2 else ""
    asset = fields[3] if len(fields) > 3 else ""
    want = "-linux.tar.xz" if LINUX else ".zip"
    check(code == 3, f"--update-check offers an update (exit code {code}, check.txt {' '.join(fields)})")
    check(newest and newest != old, f"the newest release found: {newest or 'none'}")
    check(asset.startswith("CastlevaniaCOD-PC") and asset.endswith(want) and (LINUX or "linux" not in asset),
          f"this system's package: {asset or 'none'}")

    started = time.time()
    code, fields = run(game, "--update-apply", 1200)
    check(code == 0, f"--update-apply puts it in place (exit code {code}, {round(time.time() - started)} s, "
                     f"check.txt {' '.join(fields)})")
    version = (game / "version.txt").read_text(encoding="utf-8").strip() if (game / "version.txt").exists() else ""
    check(version == newest, f"version.txt says {version or 'nothing'}")
    previous = game / "update" / "previous" / LAUNCHER
    check(previous.exists() and sha256(previous) == before, "the launcher before kept in update/previous")
    # replaced: other bytes, or (the new release's launcher the same as this one's, as 1.10.1's on Linux) another file
    now = (game / LAUNCHER).stat() if (game / LAUNCHER).exists() else None
    same = now is not None and sha256(game / LAUNCHER) == before
    check(now is not None and (not same or (now.st_ino, now.st_mtime_ns) != (before_file.st_ino, before_file.st_mtime_ns)),
          "the launcher replaced" + (" (by the same bytes: this release's launcher did not change)" if same else ""))
    if LINUX:
        check(os.access(game / LAUNCHER, os.X_OK) and os.access(game / "Castlevania", os.X_OK),
              "the new programs can run")
    leftovers = [p.name for p in (game / "update").iterdir() if p.name not in ("previous", "check.txt")]
    check(not leftovers, f"the download and unpacked folder gone ({', '.join(leftovers) or 'nothing left'})")
    for rel, data in mine.items():
        p = game / rel
        check(p.exists() and p.read_bytes() == data, f"{rel} as it was")

    print(f"update: {'passed' if not failures else 'FAILED'} ({len(failures)} failed)")
    if not failures and not keep:
        shutil.rmtree(root, ignore_errors=True)
    else:
        print(f"  the install is kept in {root}")
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
