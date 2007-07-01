#! /usr/bin/env bash
### TODO: why do we need 2 POT files for one directory?
$XGETTEXT kreadconfig.cpp -o $podir/kreadconfig.pot
$XGETTEXT kwriteconfig.cpp -o $podir/kwriteconfig.pot
