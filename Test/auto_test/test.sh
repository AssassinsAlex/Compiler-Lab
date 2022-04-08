#!/bin/bash

PARSER='./Code/parser'

# INPUT_DIR='Tests (normal)/inputs/'
# INPUT_EXT='cmm'
# EXPECT_DIR='Tests (normal)/expects/'
# EXPECT_EXT='exp'

INPUT_DIR='Tests (advanced)/inputs/'
INPUT_EXT='cmm'
EXPECT_DIR='Tests (advanced)/expects/'
EXPECT_EXT='output'

silence=0

function normalize() {
    echo "$1" | grep -E -i 'Error Type \w+ at Line [0-9]+.*' > /dev/null
    if [ $? == 0 ]; then
        echo "$1" | sed -E 's/Error Type (\w+) at Line ([0-9]+).*/\1 \2/i' | sort | sort -u -k 2,2
    else
        echo "$1" | sed -E -e '/List/d' -e '/OptTag/d' -e 's/RELOP.*/RELOP/g'
    fi
}

function a() {
    ${PARSER} "${INPUT_DIR}$1.${INPUT_EXT}"
}

function b() {
    cat "${EXPECT_DIR}$1.${EXPECT_EXT}"
}

function d() {
    local out1
    local out2
    out1=$(${PARSER} "${INPUT_DIR}$1.${INPUT_EXT}" 2>&1)
    out1=$(normalize "$out1")
    out2=$(cat "${EXPECT_DIR}/$1.${EXPECT_EXT}")
    out2=$(normalize "$out2")
    diff <(echo "$out1") <(echo "$out2") > /dev/null
    if [ $? != 0 ]; then
        echo Error $1
        if [ $silence == 0 ]; then
            diff <(echo "$out1") <(echo "$out2")
        fi
    fi
}

if [ "$1" == '-s' ]; then
    silence=1
    shift
fi

if [ "$1" == 'a' ]; then
    shift
    a $(echo $1 | sed 's/\..*//')
elif [ "$1" == 'b' ]; then
    shift
    b $(echo $1 | sed 's/\..*//')
elif [ "$1" == 'd' ]; then
    shift
    d $(echo $1 | sed 's/\..*//')
fi