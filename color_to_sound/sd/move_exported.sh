#!/usr/bin/env bash

# mv  "001 - Cdur.mp3"   "002 - Ddur(niskoA3).mp3"  "003 - Gdur.mp3"        "004 - D4.mp3"              "005 - E4.mp3"               "006 - F#4.mp3"       "007 - A3nisko.mp3"         01/
# mv  "001 - SNER.mp3"   "002 - KICK.mp3"           "003 - CLOSED HAT.mp3"  "004 - TOMB LOW.mp3"        "005 - TOMB HIGH.mp3"        "006 - OPEN HAT.mp3"  "007 - CRASH.mp3"           02/
# mv  "001 - VETAR.mp3"  "002 - NOC.mp3"            "003 - REKA.mp3"        "004 - SVETLO.mp3"          "005 - ZVEZDA.mp3"           "006 - NEBO.mp3"      "007 - TAJNA.mp3"           03/
# mv  "001 - RENDA.mp3"  "002 - BASS - DUBOKI.mp3"  "003 - VODA.mp3"        "004 - TONALNI UDARAC.mp3"  "005 - PTICE - PRIRODA.mp3"  "006 - KISA.mp3"      "007 - REVERSE EFEKAT.mp3"  04/
# mv  "001 - KRAVA.mp3"  "002 - OVCA.mp3"           "003 - KOKOSKA.mp3"     "004 - MACKA.mp3"           "005 - PAS.mp3"              "006 - ZABA.mp3"      "007 - SVINJA.mp3"          05/

for f in *.mp3; do
    # rename -f --path 's/^(\d{2})_(.+)$/$1\/$2/' "$f"
    mv -u "$f" "$(sed -r 's/^([0-9]{2})_/\1\//' <<< "$f")"
done
