Import("env")

import subprocess


def git_output(*args):
    try:
        return subprocess.check_output(
            ["git", *args], cwd=env.subst("$PROJECT_DIR"), text=True
        ).strip()
    except (OSError, subprocess.CalledProcessError):
        return "unknown"


revision = git_output("rev-parse", "--short=12", "HEAD")
dirty = git_output("status", "--porcelain", "--untracked-files=no")
build_id = revision + ("-dirty" if dirty and dirty != "unknown" else "")
env.Append(CPPDEFINES=[("FIRMWARE_BUILD_ID", env.StringifyMacro(build_id))])
