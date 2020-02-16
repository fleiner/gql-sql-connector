#!/usr/bin/python

## \file
# \brief get a new copy of the postgresql type catalog
# \author Claudio Fleiner
# \copyright (C) Claudio Fleiner 2018
#
# It cannot be fully queried from a running postgresql db.
# pg_type is not complete for example and oid 1043 (one of the varchar
# types) is missing. Therefore download the source and create an internal
# table. This is done on demand only and not part of the regular build

import os
import requests
import re


r=requests.get("https://raw.githubusercontent.com/postgres/postgres/master/src/include/catalog/pg_type.dat")

# we doe very simple parsing, assume a type start with '{' ends with '}' and
# we only care about oid and typname. For all known typename create a hash
# table to map oid to typename.


known_types={
        "numeric":"FLOAT",
        "int8":"INTEGER",
        "int4":"INTEGER",
        "int2":"INTEGER",
        "bool":"BOOL",
        "float4":"FLOAT",
        "float8":"FLOAT",
        "timestamp":"DATETIME",
        "timestamptz":"DATETIME",
        "date":"DATE",
        "time":"TIME",
}


typere=re.compile(r"{\s*oid\s*=>\s*'(\d+)'[^}]*typname\s*=>\s*'([^']*)[^}]*}")
for m in typere.finditer(r.text):
    oid=int(m.group(1))
    name=m.group(2)
    if name in known_types:
        print "{",oid,",PG_TYPES::"+known_types[name]+"},",
    elif name[0]=='_' and name[1:] in known_types:
        print "{",oid,",PG_TYPES::"+known_types[name[1:]]+"},",
    else:
        print "{",oid,",PG_TYPES::STRING},",
    print "/*",name,"*/"
