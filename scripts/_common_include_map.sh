#!/usr/bin/env bash
# Generate awk replace script: maps every relocated Common header to its new path.
# Usage: source this to get $REPLACE_SED with -e arguments for sed.
set -e

ROOT="D:/Projects/MMORPG/TalesOfPirate/sources/Libraries/Common"

declare -A FILE_DOMAIN

for d in Database Core Crypto Discord Network Character Item Inventory Skill NPC World Audio Effect Mount Progression Localization Server Misc; do
  for f in "$ROOT/include/$d"/*.h; do
    [ -f "$f" ] || continue
    base=$(basename "$f")
    FILE_DOMAIN[$base]="$d"
  done
done

# Output: header_name<TAB>domain
for f in "${!FILE_DOMAIN[@]}"; do
  echo -e "${f}\t${FILE_DOMAIN[$f]}"
done | sort
