#!/usr/bin/env python3
"""Wait for CI run on current HEAD; on failure, fetch + print compiler errors.

Reads GITHUB_TOKEN from env to download job logs via the API. Does NOT auto-fix.
"""
import json, os, re, subprocess, sys, time, urllib.request, urllib.error

REPO = "CarterH801/mayhem-firmware"
TOKEN = os.environ.get("GITHUB_TOKEN")
HDRS = {"Accept": "application/vnd.github+json"}
if TOKEN:
    HDRS["Authorization"] = f"token {TOKEN}"

def api(url):
    return json.loads(urllib.request.urlopen(urllib.request.Request(url, headers=HDRS)).read())

class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, *a, **k): return None

def fetch_log(job_id):
    """API endpoint 302s to a SAS blob URL; do not forward Authorization."""
    op = urllib.request.build_opener(NoRedirect)
    url = f"https://api.github.com/repos/{REPO}/actions/jobs/{job_id}/logs"
    try:
        op.open(urllib.request.Request(url, headers=HDRS))
    except urllib.error.HTTPError as e:
        if e.code in (301, 302, 303, 307):
            loc = e.headers.get("Location")
            return urllib.request.urlopen(loc).read().decode("utf-8", "replace")
        raise
    return ""

def main():
    target = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
    print(f"Waiting for CI run on {target[:8]}", flush=True)
    seen = None
    while True:
        try:
            run = api(f"https://api.github.com/repos/{REPO}/actions/runs?branch=next&per_page=1")["workflow_runs"][0]
        except Exception as e:
            print(f"[err] {e}", flush=True); time.sleep(30); continue

        if run["head_sha"] != target:
            time.sleep(15); continue

        rid, status, concl, sha = run["id"], run["status"], run["conclusion"], run["head_sha"][:8]
        if rid != seen:
            print(f"[run {rid}] {sha} status={status} conclusion={concl}", flush=True)
            seen = rid

        if status == "completed":
            print(f"\n=== DONE: {concl} ===")
            print(f"Run: {run['html_url']}")
            jobs = api(f"https://api.github.com/repos/{REPO}/actions/runs/{rid}/jobs")["jobs"]
            for j in jobs:
                print(f"  [{j['conclusion']}] {j['name']}: {j['html_url']}")
            if concl == "success":
                print("\nAll green.")
                return

            if not TOKEN:
                print("\nNo GITHUB_TOKEN; cannot auto-fetch logs.")
                return

            for j in jobs:
                if j["conclusion"] != "failure": continue
                print(f"\n--- ERRORS from {j['name']} ---")
                try:
                    log = fetch_log(j["id"])
                except Exception as e:
                    print(f"[fetch err] {e}"); continue
                strip_ansi = lambda s: re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", s)
                errs = [strip_ansi(l) for l in log.split("\n") if "error:" in l.lower()]
                print(f"({len(errs)} errors)")
                for l in errs[:60]:
                    print(l)
                break  # one job's errors usually enough; they overlap
            return

        time.sleep(30)

if __name__ == "__main__":
    sys.exit(main())
