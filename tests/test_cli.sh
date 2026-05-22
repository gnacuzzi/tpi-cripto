#!/usr/bin/env bash
set -euo pipefail

BIN="${1:?usage: test_cli.sh <path-to-visualSSS>}"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

pass=0
fail=0

assert_fails() {
    local name="$1"
    shift
    if "$@" >/dev/null 2>&1; then
        echo "[FAIL] $name (expected failure)"
        fail=$((fail + 1))
    else
        echo "[OK] $name"
        pass=$((pass + 1))
    fi
}

assert_ok() {
    local name="$1"
    shift
    if "$@" >/dev/null 2>&1; then
        echo "[OK] $name"
        pass=$((pass + 1))
    else
        echo "[FAIL] $name (expected success)"
        fail=$((fail + 1))
    fi
}

assert_stderr_contains() {
    local name="$1"
    local needle="$2"
    shift 2
    local output
    output="$("$@" 2>&1 >/dev/null || true)"
    if echo "$output" | grep -Fq "$needle"; then
        echo "[OK] $name"
        pass=$((pass + 1))
    else
        echo "[FAIL] $name (stderr missing '$needle')"
        echo "$output"
        fail=$((fail + 1))
    fi
}

touch "$TMP_DIR/secret.bmp"
touch "$TMP_DIR/c1.bmp" "$TMP_DIR/c2.bmp" "$TMP_DIR/c3.bmp"

assert_fails "no args" "$BIN"
assert_stderr_contains "no args shows usage" "usage:" "$BIN"
assert_stderr_contains "unknown flag" "unknown argument" "$BIN" -d -secret x.bmp -k 2 -foo bar
assert_stderr_contains "missing mode" "missing mode" "$BIN" -secret x.bmp -k 2
assert_stderr_contains "both modes" "mode already specified" "$BIN" -d -r -secret x.bmp -k 2
assert_stderr_contains "missing secret" "missing -secret" "$BIN" -d -k 2
assert_stderr_contains "missing k" "missing -k" "$BIN" -d -secret x.bmp
assert_stderr_contains "distribute missing secret file" "secret file does not exist" \
    "$BIN" -d -secret "$TMP_DIR/missing.bmp" -k 2 -dir "$TMP_DIR"
assert_stderr_contains "k=1 rejected" "k must be between 2 and 10" \
    "$BIN" -d -secret "$TMP_DIR/secret.bmp" -k 1 -dir "$TMP_DIR"
assert_stderr_contains "k=0 rejected" "k must be between 2 and 10" \
    "$BIN" -d -secret "$TMP_DIR/secret.bmp" -k 0 -dir "$TMP_DIR"
assert_stderr_contains "k=11 rejected" "k must be between 2 and 10" \
    "$BIN" -d -secret "$TMP_DIR/secret.bmp" -k 11 -dir "$TMP_DIR"
assert_stderr_contains "k greater than n" "k cannot be greater than n" \
    "$BIN" -d -secret "$TMP_DIR/secret.bmp" -k 5 -n 3 -dir "$TMP_DIR"

assert_ok "enunciado distribute example" \
    "$BIN" -d -secret "$TMP_DIR/secret.bmp" -k 2 -n 4 -dir "$TMP_DIR"
assert_ok "enunciado distribute k=3 cwd" \
    "$BIN" -d -secret "$TMP_DIR/secret.bmp" -k 3 -dir "$TMP_DIR"
assert_ok "enunciado recover example" \
    "$BIN" -r -secret "$TMP_DIR/out.bmp" -k 2 -dir "$TMP_DIR"
assert_ok "enunciado recover k=3 cwd" \
    "$BIN" -r -secret "$TMP_DIR/out.bmp" -k 3 -dir "$TMP_DIR"

echo "CLI tests: $pass passed, $fail failed"
if [ "$fail" -ne 0 ]; then
    exit 1
fi
