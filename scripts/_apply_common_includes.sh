#!/usr/bin/env bash
# Replace #include "X.h" → #include "Domain/X.h" in given files,
# where X.h is any header relocated to Common/include/<Domain>/.
set -e

MAP_FILE="$1"      # tsv: header<TAB>domain
shift
TARGETS=("$@")     # files to patch

[ -f "$MAP_FILE" ] || { echo "map file not found: $MAP_FILE"; exit 1; }

# Build a single sed script
SED_SCRIPT=$(mktemp)
trap 'rm -f "$SED_SCRIPT"' EXIT

while IFS=$'\t' read -r header domain; do
  [ -z "$header" ] && continue
  printf 's|#include "%s"|#include "%s/%s"|gI\n' "$header" "$domain" "$header" >> "$SED_SCRIPT"
  printf 's|#include <%s>|#include "%s/%s"|gI\n' "$header" "$domain" "$header" >> "$SED_SCRIPT"
  # Also handle "./X.h" and ".\X.h" relative forms
  printf 's|#include "\\./%s"|#include "%s/%s"|gI\n' "$header" "$domain" "$header" >> "$SED_SCRIPT"
  printf 's|#include "\\.\\\\%s"|#include "%s/%s"|gI\n' "$header" "$domain" "$header" >> "$SED_SCRIPT"
done < "$MAP_FILE"

echo "Generated $(wc -l < "$SED_SCRIPT") replacement rules"

count=0
for f in "${TARGETS[@]}"; do
  if [ -f "$f" ]; then
    before=$(md5sum "$f" | cut -d' ' -f1)
    sed -i -f "$SED_SCRIPT" "$f"
    after=$(md5sum "$f" | cut -d' ' -f1)
    if [ "$before" != "$after" ]; then
      count=$((count+1))
    fi
  fi
done
echo "Patched $count of ${#TARGETS[@]} files"
