#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
make clean
make

log=$(mktemp)
trap 'rm -f "$log"' EXIT

# Allow the boot-time process/timer demonstration to finish, exercise the
# interactive shell, then send QEMU's Ctrl+A X escape sequence.
{ sleep 8; printf 'help\rprasanthi\rwhoami\rstage\rabout\rpwd\rps\rgetpid\rticks\ruptime\runame\rls\rcat about.txt\rcat missing.txt\risolation\rhello\rclear\r'; sleep 1; printf '\001x'; } |
    timeout 30s make qemu >"$log" 2>&1

cat "$log"
grep -q 'Sv39 virtual memory enabled' "$log"
grep -q 'Timer preempt:' "$log"
grep -q 'Wait returned child PID' "$log"
grep -q 'Fork child returned 0' "$log"
grep -q 'Timer woke a sleeping process' "$log"
grep -q 'NanoBoot user shell (U mode)' "$log"
grep -q 'User stack permission matrix: PASS' "$log"
grep -q 'PID 1' "$log"
grep -q 'Prasanthi Lakshmi' "$log"
grep -q 'Stage 9:' "$log"
grep -q 'welcome.txt' "$log"
grep -q 'cat: file not found' "$log"
echo 'NanoBoot demo checks passed.'
