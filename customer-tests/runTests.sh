#!/bin/bash -e

make -C ../src
PATH=../src:$PATH python test_actions.py
