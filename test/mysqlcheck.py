#!/usr/bin/python

"""@package mysql check code
\author Claudio Fleiner
"""

import argparse
import json
import sys
import codecs
import os
import traceback
import subprocess

def commandline_arg(bytestring):
    """ handler to deal with utf-8 encoded arguments """

    try:
        unicode_string = bytestring.decode('utf8')
    except Exception, err:
        traceback.print_exc(file=sys.stdout)
        traceback.print_exc(limit=1, file=sys.stdout)
        raise

    return unicode_string


url="mysql://gqltest:gqltest@localhost:3306/gqltest"
if os.path.basename(sys.argv[0])=="postgresqlcheck.py":
    url="postgresql://gqltest:gqltest@localhost:5432/gqltest"

if "GQLURL" in os.environ:
    url=os.environ["GQLURL"]
parser = argparse.ArgumentParser()
parser.add_argument("-u","--url", help="connection url, $GQLULR if defined", default=url)
parser.add_argument("-d","--default", help="default table", default="all")
parser.add_argument("-t","--tables", help="list of accessible tables", default="all")
parser.add_argument("-c","--column", action="append", help="one per column: id,label,type,pattern")
parser.add_argument("-l","--locale", help="set locale")
parser.add_argument("-e","--extended", help="allow extended functions", action="store_true")
parser.add_argument("-v","--verbose", help="print result", action="store_true")
parser.add_argument("query", help="gql query")
parser.add_argument("value", nargs="*", type=commandline_arg, help="expected results, v and f if both are expected")

sys.stdout = codecs.getwriter('utf-8')(sys.stdout);

args = parser.parse_args()

cmd=["../gqldb"]
if args.locale:
    cmd+=["-l",args.locale]
if args.tables:
    cmd+=["-t",args.tables]
if args.extended:
    cmd+=["-e"]
cmd+=["-d",args.default,args.url,args.query]

if args.verbose:
    print "CMD:","'"+"' '".join(cmd)+"'"
    sys.stdout.flush()

res=json.loads(subprocess.check_output(cmd))

if args.verbose:
    print "RES:",json.dumps(res,indent=4)
    sys.stdout.flush()

assert res["status"]=="ok","query failed:"+res["message"]
assert len(res["table"]["cols"])==len(args.column),"expected %r colums, got %r"%(len(args.column),len(res["table"]["cols"]))

cols=res["table"]["cols"]

for c in range(len(args.column)):
    n=args.column[c].split(",",3)
    assert len(n)==4,"col %r: expected id,label,type,pattern but got \"%s\""%(c+1,args.column[c])
    assert cols[c]["id"]==n[0].replace("%2C",","),"col %r: expected id \"%r\" but got \"%r\""%(c+1,n[0].replace("%2C",","),cols[c]["id"])
    assert cols[c]["label"]==n[1].replace("%2C",","),"col %r: expected id \"%r\" but got \"%r\""%(c+1,n[1].replace("%2C",","),cols[c]["label"])
    assert cols[c]["type"]==n[2],"col %r: expected id \"%r\" but got \"%r\""%(c+1,n[2],cols[c]["type"])
    assert cols[c]["pattern"]==n[3],"col %r: expected id \"%r\" but got \"%r\""%(c+1,n[3],cols[c]["pattern"])

rows=res["table"]["rows"]
cnt=0
for l in range(len(rows)):
    for c in range(len(rows[l]["c"])):
        cell=rows[l]["c"][c]
        if "v" in cell:
            assert cnt<len(args.value),"not enough values: %r instead of %r"%(len(args.value)-1,cnt)
            if type(cell["v"])==str or type(cell["v"])==unicode:
                assert cell["v"]==args.value[cnt],"row %r, col %r: expected \"%r\" but got \"%r\""%(l+1,c+1,args.value[cnt],cell["v"])
            else:
                assert unicode(cell["v"])==args.value[cnt],"row %r, col %r: expected \"%r\" but got \"%r\""%(l+1,c+1,args.value[cnt],unicode(cell["v"]))
            cnt+=1
        if "f" in cell:
            assert cnt<len(args.value),"not enough values"
            assert unicode(cell["f"])==args.value[cnt],"row %r, col %r: expected \"%r\" but got \"%r\""%(l+1,c+1,args.value[cnt],unicode(cell["f"]))
            cnt+=1

assert cnt==len(args.value),"too many values, %r used but got %r"%(cnt,len(args.value))
