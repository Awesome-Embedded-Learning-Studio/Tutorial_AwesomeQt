#!/usr/bin/env python3
"""Check reachability of all links in tutorial/ markdown files."""

import argparse
import re
import ssl
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError

# Match markdown links: [text](url), skip images ![alt](url)
LINK_RE = re.compile(r"(?<!!)\[([^\]]*)\]\(([^)]+)\)")

# Match external URLs
EXTERNAL_RE = re.compile(r"^https?://", re.IGNORECASE)

# ANSI colors
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
CYAN = "\033[36m"
BOLD = "\033[1m"
RESET = "\033[0m"


def find_markdown_files(root: Path) -> list[Path]:
    return sorted(root.rglob("*.md"))


def strip_code_blocks(text: str) -> str:
    """Remove fenced code blocks (```...```) and inline code (`...`) from text."""
    # Remove fenced code blocks (``` ... ```)
    text = re.sub(r"```.*?```", "", text, flags=re.DOTALL)
    # Remove inline code (` ... `)
    text = re.sub(r"`[^`]+`", "", text)
    return text


def extract_links(file_path: Path) -> list[tuple[str, str]]:
    """Return list of (link_text, url) from a markdown file."""
    text = file_path.read_text(encoding="utf-8", errors="replace")
    text = strip_code_blocks(text)
    links = []
    for match in LINK_RE.finditer(text):
        link_text = match.group(1).strip()
        url = match.group(2).strip()
        # Skip pure anchors like #section-title
        if url.startswith("#"):
            continue
        # Skip URLs containing spaces or newlines (not valid URLs, likely code)
        if " " in url or "\n" in url:
            continue
        links.append((link_text, url))
    return links


def check_external_url(url: str, timeout: int) -> tuple[bool, str]:
    """Check if an external URL is reachable. Returns (ok, status_message)."""
    # Strip fragment for HTTP request
    clean_url = url.split("#")[0]
    req = Request(clean_url, method="HEAD", headers={
        "User-Agent": "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    })
    try:
        with urlopen(req, timeout=timeout) as resp:
            status = resp.status
            if 200 <= status < 400:
                return True, str(status)
            return False, f"HTTP {status}"
    except HTTPError as e:
        # Some servers reject HEAD, retry with GET
        if e.code in (404, 405, 406):
            try:
                req.method = "GET"
                with urlopen(req, timeout=timeout) as resp:
                    if 200 <= resp.status < 400:
                        return True, str(resp.status)
                    return False, f"HTTP {resp.status}"
            except Exception as e2:
                return False, str(e2)
        if 200 <= e.code < 400:
            return True, str(e.code)
        return False, f"HTTP {e.code}"
    except Exception as e:
        err_msg = str(e)
        # Retry once on transient SSL / network errors
        if "SSL" in err_msg or "timed out" in err_msg or "EOF" in err_msg:
            try:
                # Use an unverified SSL context as fallback
                ctx = ssl.create_default_context()
                ctx.check_hostname = False
                ctx.verify_mode = ssl.CERT_NONE
                req.method = "GET"
                with urlopen(req, timeout=timeout, context=ctx) as resp:
                    if 200 <= resp.status < 400:
                        return True, str(resp.status)
            except Exception:
                pass
        return False, err_msg


def check_relative_path(link_url: str, source_file: Path) -> tuple[bool, str]:
    """Check if a relative file path exists. Returns (ok, message)."""
    # Strip fragment
    target = link_url.split("#")[0]
    resolved = (source_file.parent / target).resolve()
    if resolved.exists():
        return True, "OK"
    return False, f"file not found: {resolved}"


def main():
    parser = argparse.ArgumentParser(
        description="Check reachability of links in tutorial markdown files."
    )
    parser.add_argument(
        "path",
        nargs="?",
        default="tutorial",
        help="Root directory to scan (default: tutorial)",
    )
    parser.add_argument("--timeout", type=int, default=10, help="HTTP timeout in seconds (default: 10)")
    parser.add_argument("--workers", type=int, default=8, help="Concurrent HTTP workers (default: 8)")
    args = parser.parse_args()

    root = Path(args.path)
    if not root.is_dir():
        print(f"{RED}Error: {root} is not a directory{RESET}")
        sys.exit(2)

    md_files = find_markdown_files(root)
    if not md_files:
        print(f"{YELLOW}No markdown files found under {root}{RESET}")
        sys.exit(0)

    # Collect all links: (source_file, link_text, url)
    all_links: list[tuple[Path, str, str]] = []
    for f in md_files:
        for text, url in extract_links(f):
            all_links.append((f, text, url))

    external = [(f, t, u) for f, t, u in all_links if EXTERNAL_RE.match(u)]
    relative = [(f, t, u) for f, t, u in all_links if not EXTERNAL_RE.match(u)]

    print(f"{BOLD}Link Checker{RESET}")
    print(f"  Markdown files : {len(md_files)}")
    print(f"  Total links    : {len(all_links)}")
    print(f"  External URLs  : {len(external)}")
    print(f"  Relative paths : {len(relative)}")
    print()

    failures: list[tuple[Path, str, str, str]] = []

    # --- Check relative paths ---
    print(f"{CYAN}[1/2] Checking relative file paths...{RESET}")
    rel_ok = 0
    for f, text, url in relative:
        ok, msg = check_relative_path(url, f)
        if ok:
            rel_ok += 1
        else:
            failures.append((f, text, url, msg))
    print(f"  {GREEN}{rel_ok} OK{RESET}, {RED}{len(relative) - rel_ok} broken{RESET}")

    # --- Check external URLs concurrently ---
    print(f"{CYAN}[2/2] Checking external URLs ({args.workers} workers)...{RESET}")
    ext_ok = 0
    ext_done = 0

    def _check(item):
        f, text, url = item
        ok, msg = check_external_url(url, args.timeout)
        return f, text, url, ok, msg

    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(_check, item): item for item in external}
        for future in as_completed(futures):
            ext_done += 1
            f, text, url, ok, msg = future.result()
            if ok:
                ext_ok += 1
            else:
                failures.append((f, text, url, msg))
            # Progress line
            pct = ext_done * 100 // len(external) if external else 100
            print(f"\r  [{ext_done}/{len(external)}] {pct}%", end="", flush=True)

    if external:
        print()
    print(f"  {GREEN}{ext_ok} OK{RESET}, {RED}{len(external) - ext_ok} failed{RESET}")

    # --- Summary ---
    print()
    total_ok = rel_ok + ext_ok
    print(f"{BOLD}{'=' * 50}{RESET}")
    print(f"  Total   : {len(all_links)}")
    print(f"  {GREEN}Passed  : {total_ok}{RESET}")
    print(f"  {RED}Failed  : {len(failures)}{RESET}")

    if failures:
        print(f"\n{BOLD}{RED}Broken links:{RESET}")
        for f, text, url, msg in failures:
            rel_path = f.relative_to(Path.cwd()) if f.is_relative_to(Path.cwd()) else f
            print(f"  {RED}✗{RESET} {rel_path}")
            print(f"    text : {text}")
            print(f"    url  : {url}")
            print(f"    error: {msg}")
        sys.exit(1)
    else:
        print(f"\n{GREEN}All links are reachable!{RESET}")
        sys.exit(0)


if __name__ == "__main__":
    main()
