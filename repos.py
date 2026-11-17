#!/usr/bin/env python3
"""Run git across every Codaphela repository.

    python repos.py status            Show git status.
    python repos.py pull              Pull, rebasing local commits.
    python repos.py push              Push.
    python repos.py sync              Pull then push.  Doesn't push if any pull failed.
    python repos.py commit <message>  Stage everything (including deletions) and commit it where there are changes.
    python repos.py link              Make repos, a link to this file, in the folder the repositories are in.
    python repos.py clone             Clone every repository that isn't there, from where CommonLibraries came from.

The repositories are the folders beside CommonLibraries, which this file is in, so it runs from anywhere:
python CommonLibraries/repos.py status from the folder they are all in, or, once linked, ./repos status there.

Pull and push run on every repository at once, each repository's output shown in one block as it completes.
Carries on after a repository fails and exits non-zero if any did.
"""

import argparse
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

# Directories beside CommonLibraries, which this file is in, and what each repository is called where it's cloned from.
REPOS = {
    "CommonLibraries": "Codaphela.Library",
    "HCP65": "HCP65",
    "HCP65-IDE": "HCP65-IDE",
    "HCP65-Sculpt": "HCP65-Sculpt",
    "IntoOblivionX": "IntoOblivionX",
    "LogiCim": "Codaphela.LogiCim",
    "Pico": "Pico",
    "StarControlRedux": "StarControlRedux",
    "WindowInputTester": "WindowInputTester",
}

HOME_REPO = "CommonLibraries"  # The repository this file is in.

ROOT = Path(__file__).resolve().parent.parent


def git(repo, *args):
    """Runs git in the repository, returning True if it succeeded."""
    return subprocess.run(["git", "-C", str(ROOT / repo), *args]).returncode == 0


def existing_repos():
    repos = []
    for repo in REPOS:
        if (ROOT / repo / ".git").exists():
            repos.append(repo)
        else:
            print(f"{repo} is not a git repository in {ROOT}, skipped.", file=sys.stderr)
    return repos


def for_each_repo(*args):
    """Runs the same git command in every repository, returning True if all succeeded."""
    succeeded = True
    for repo in existing_repos():
        print(f"=== {repo}", flush=True)
        succeeded = git(repo, *args) and succeeded
        print(flush=True)
    return succeeded


def git_captured(repo, *args):
    """Runs git in the repository, returning whether it succeeded and its stdout and stderr together as bytes."""
    result = subprocess.run(
        ["git", "-C", str(ROOT / repo), *args],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return result.returncode == 0, result.stdout


def for_each_repo_concurrently(*args):
    """Runs the same git command in every repository at once, showing each one's output as a block as it completes.
    Returns True if all succeeded."""
    repos = existing_repos()
    failed = []
    with ThreadPoolExecutor(max_workers=len(repos) or 1) as executor:
        futures = {executor.submit(git_captured, repo, *args): repo for repo in repos}
        for future in as_completed(futures):
            repo = futures[future]
            succeeded, output = future.result()
            if not succeeded:
                failed.append(repo)
            print(f"=== {repo}" + ("" if succeeded else " (FAILED)"), flush=True)
            # Written as bytes as the console's encoding might not be able to represent what git said.
            sys.stdout.buffer.write(output)
            sys.stdout.buffer.flush()
            print(flush=True)
    if failed:
        print(f"=== Failed: {', '.join(sorted(failed))}", flush=True)
    return not failed


def commit(message):
    succeeded = True
    for repo in existing_repos():
        print(f"=== {repo}", flush=True)
        if not git(repo, "add", "-A"):
            succeeded = False
            print(flush=True)
            continue
        if git(repo, "diff", "--cached", "--quiet"):
            print("Nothing to commit.", flush=True)
        else:
            succeeded = git(repo, "commit", "-m", message) and succeeded
        git(repo, "status", "--short")
        print(flush=True)
    return succeeded


def link():
    """Makes repos, a relative link to this file, in the folder the repositories are in.  Returns True if it's there."""
    path = ROOT / "repos"
    target = Path(__file__).resolve().relative_to(ROOT)
    if path.is_symlink():
        if Path(os.readlink(path)) == target:
            print(f"{path} is already linked to {target}.")
            return True
        path.unlink()
    elif path.exists():
        print(f"{path} exists and isn't a link, left alone.", file=sys.stderr)
        return False
    try:
        path.symlink_to(target)
    except OSError as error:
        # Windows makes links only for administrators or in Developer Mode.
        print(f"Couldn't link {path} to {target}: {error}", file=sys.stderr)
        return False
    print(f"Linked {path} to {target}.")
    return True


def remote_url(name):
    """Where the repository called name is cloned from: beside CommonLibraries' own origin, so by the same protocol,
    host and account as it was.  None if CommonLibraries' origin isn't its name where it's expected."""
    result = subprocess.run(
        ["git", "-C", str(ROOT / HOME_REPO), "remote", "get-url", "origin"], stdout=subprocess.PIPE, text=True
    )
    origin = result.stdout.strip()
    for suffix in (".git", ""):
        ending = REPOS[HOME_REPO] + suffix
        if result.returncode == 0 and origin.endswith(ending):
            return origin[: -len(ending)] + name + suffix
    return None


def clone():
    """Clones every repository that isn't there.  Returns True if every one is there afterwards."""
    succeeded = True
    for repo, name in REPOS.items():
        path = ROOT / repo
        if (path / ".git").exists():
            continue
        print(f"=== {repo}", flush=True)
        url = remote_url(name)
        if url is None:
            print(f"{HOME_REPO}'s origin doesn't end in {REPOS[HOME_REPO]}: can't tell where to clone from.", file=sys.stderr)
            succeeded = False
        elif path.exists() and any(path.iterdir()):
            print(f"{path} exists and isn't a git repository, left alone.", file=sys.stderr)
            succeeded = False
        else:
            succeeded = subprocess.run(["git", "clone", url, str(path)]).returncode == 0 and succeeded
        print(flush=True)
    if succeeded:
        print("Every repository is cloned.")
    return succeeded


def pull():
    return for_each_repo_concurrently("pull", "--rebase")


def push():
    return for_each_repo_concurrently("push")


def main():
    parser = argparse.ArgumentParser(description="Run git across every Codaphela repository.")
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("status", help="show git status")
    commands.add_parser("pull", help="pull, rebasing local commits")
    commands.add_parser("push", help="push")
    commands.add_parser("sync", help="pull then push; doesn't push if any pull failed")
    commit_parser = commands.add_parser("commit", help="stage everything and commit it where there are changes")
    commit_parser.add_argument("message", nargs="+", help="the commit message")
    commands.add_parser("link", help="make repos, a link to this file, in the folder the repositories are in")
    commands.add_parser("clone", help="clone every repository that isn't there")
    args = parser.parse_args()

    if args.command == "status":
        succeeded = for_each_repo("status")
    elif args.command == "pull":
        succeeded = pull()
    elif args.command == "push":
        succeeded = push()
    elif args.command == "sync":
        succeeded = pull() and push()
    elif args.command == "link":
        succeeded = link()
    elif args.command == "clone":
        succeeded = clone()
    else:
        succeeded = commit(" ".join(args.message))
    return 0 if succeeded else 1


if __name__ == "__main__":
    sys.exit(main())
