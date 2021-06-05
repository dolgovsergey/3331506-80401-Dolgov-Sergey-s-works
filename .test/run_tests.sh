#!/bin/bash

echo "########################################"
echo "Running tests"

chmod +x test_runner && ./test_runner

retval=$?

if [ "$retval" -ne "0" ]; then
   echo -e "\e[41mTests failed!\e[0m"
   exit 1;
fi

echo "Tests completed"
