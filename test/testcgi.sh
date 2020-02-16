#!/bin/bash

set -e -x
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}
mkdir -p cgi-bin

cp -af "$(realpath "$srcdir")"/*.cgi cgi-bin/
{
    echo "#!$(realpath ..)/gqldb"
    cat  "$srcdir/gqlconfig.cgi"
} > cgi-bin/gqlconfig.cgi
chmod 755 cgi-bin/gqlconfig.cgi

rm -f cgi.log


GLOG_dir=. GLOG_minloglevel=0 python -u -m CGIHTTPServer 0 > cgi.log 2>&1 &
pid="$!"

trap "set +x ; kill -9 $pid ; wait" EXIT

cnt=0
while ! grep -q 'Serving HTTP' cgi.log && kill -0 $pid  ; do
    sleep 1
done
port=$(sed -n 's/Serving HTTP.* port \([0-9]*\).*/\1/p' cgi.log)
echo "PORT: $port"

for i in "$srcdir/"goldcgi.*.url ; do
    hfile="$srcdir/$(basename "$i" .url).header"
    cfile="$srcdir/$(basename "$i" .url).config"
    dumpfile="$(basename "$i" .url).dump"
    docfile="$(basename "$i" .url).doc"
    url=$(head -1 "$i")
    cmd=
    if [ -f "$cfile" ] ; then cmd="-K $cfile" ; fi

    hname=$(basename "$i" .url)
    curl $cmd -D "$dumpfile" "http://127.0.0.1:$port$url" > "$docfile"
    for line in $(seq 2 $(wc -l < $hfile)) ; do
        fgrep "$(head -$line $hfile | tail -1)" "$dumpfile"
    done

    bn=$(basename $i .url)
    if grep javascript "$dumpfile" ; then
        sed -n 's/\([^(]*\)(\(.*\));/{"handler":"\1","res":\2}/p' "$docfile" | \
        python -c 'import json,sys;print json.dumps(json.loads(sys.stdin.read()),sort_keys=True,indent=4)' > "$docfile.json"
        cmp "$docfile.json" "$srcdir/$bn.json"
    else
        cmp "$docfile" "$srcdir/$bn.doc"
    fi
done
