#!/bin/bash

export HTTP_ACCEPT_LANGUAGE=de-CH
# python cgi does not pass through langauges, so set it here manually
../gqldb -d alltypes mysql://gqltest:gqltest@localhost/gqltest 2>&1
