#!/bin/sh
# test
./mailsend.exe -smtp $1 -domain muquit.com -t muquit@localhost +cc +bc -sub test -from muquit@muquit.com -a test.gif,image/gif
