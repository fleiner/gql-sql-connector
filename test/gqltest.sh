#!/bin/bash

set -e -x 
srcdir=${srcdir:-$(dirname $0)}
url="mysql://gqltest:gqltest@localhost:3306/gqltest"
purl="postgresql://gqltest:gqltest@localhost:5432/gqltest"

test "$(../gqldb -d T1 "$url" 'select EINS order by EINS options no_format')" = '{"status":"ok","table":{"cols":[{"format":null,"id":"EINS","label":"","pattern":"General","type":"number"}],"rows":[{"c":[{"v":2}]},{"c":[{"v":3}]},{"c":[{"v":5}]},{"c":[{"v":7}]}]},"version":"0.7"}'

test "$(../gqldb -d T1 --tables alltypes "$url" 'select `alltypes.int` order by `alltypes.int` options no_format')" = '{"status":"ok","table":{"cols":[{"format":null,"id":"alltypes.int","label":"","pattern":"General","type":"number"}],"rows":[{"c":[{"v":0}]},{"c":[{"v":123123123}]},{"c":[{"v":2000000000}]},{"c":[{"v":4294967295}]}]},"version":"0.7"}'

test "$(../gqldb -d T1 -l de_CH.utf8 --tables alltypes "$url" 'select `alltypes.int` order by `alltypes.int` options no_values')" = '{"status":"ok","table":{"cols":[{"format":null,"id":"alltypes.int","label":"","pattern":"General","type":"number"}],"rows":[{"c":[{"f":"0"}]},{"c":[{"f":"123'\''123'\''123"}]},{"c":[{"f":"2'\''000'\''000'\''000"}]},{"c":[{"f":"4'\''294'\''967'\''295"}]}]},"version":"0.7"}'


test "$(../gqldb "$srcdir/gqlconfig.1" 'select `alltypes.tinyint` where `alltypes.t1key`=`T B.Ints` order by tinyint')" = '{"status":"ok","table":{"cols":[{"format":null,"id":"alltypes.tinyint","label":"","pattern":"General","type":"number"}],"rows":[{"c":[{"f":"0","v":0}]},{"c":[{"f":"127","v":127}]}]},"version":"0.7"}'
test "$(../gqldb "$srcdir/gqlconfig.2" 'select `alltypes.tinyint` where `alltypes.t1key`=`T B.ints` order by `alltypes.tinyint`')" = '{"status":"ok","table":{"cols":[{"format":null,"id":"alltypes.tinyint","label":"","pattern":"General","type":"number"}],"rows":[{"c":[{"f":"0","v":0}]},{"c":[{"f":"127","v":127}]}]},"version":"0.7"}'


../gqldb "$url" 'select `alltypes.int`' -d T1 | grep 'does not exists or is not accessible'

../gqldb -d T1 "mysql://gqltest:gqltest@127.0.0.1:3706/gqltest" 'select *' | grep 'db connection failed'
../gqldb -d T1 "mysql://gqltest:gqlte@127.0.0.1:3706/gqltest" 'select *' | grep 'db connection failed'

../gqldb -d T1 "postgresql://gqltest:gqltest@127.0.0.1:5431/gqltest" 'select *' | grep 'db connection failed'
../gqldb -d T1 "postgresql://gqltest:gqlte@127.0.0.1:5432/gqltest" 'select *' | grep 'db connection failed'


exit 0
