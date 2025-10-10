#!/usr/bin/env bash
rm -f */*.mp3
for f in *.mp3; do
    # rename -f --path 's/^(\d{2})_(.+)$/$1\/$2/' "$f"
    mv -u "$f" "$(sed -r 's/^([0-9]{2})_/\1\//' <<< "$f")"
done
