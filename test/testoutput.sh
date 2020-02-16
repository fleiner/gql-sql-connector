#!/bin/bash

set -e -x
export LC_ALL=C

srcdir=${srcdir:-$(dirname $0)}

for i in csv tsv html ; do
    ../gqldb -f "$i" -d alltypes  mysql://gqltest:gqltest@localhost/gqltest 'select `text`,`varchar`,`bigint` order by `int` format `bigint` "#,###"' > test.$i
    cmp "test.$i" "$srcdir/gold.$i"
done

../gqldb -f json -d alltypes mysql://gqltest:gqltest@localhost/gqltest 'select `text`,`varchar`,`bigint` order by `int` format `bigint` "#,###"' | \
    python -c 'import json,sys;print json.dumps(json.loads(sys.stdin.read()),sort_keys=True,indent=4)' > test.json
cmp "test.json" "$srcdir/gold.json"

