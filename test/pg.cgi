#!/bin/bash

exec ../gqldb -d alltypes postgresql://gqltest:gqltest@localhost/gqltest 2>&1
