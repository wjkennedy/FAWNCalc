#!/bin/bash
#
# FAWNCalc build
#
####################

sh verbump.sh

./configure

make

echo "---------------------------------------"
if [ $? -eq 0 ]
then
echo "FAWNCalc built."
else
echo "fail."
fi
