
# print your parser's output
./test.sh a L1-A-0-1

# print answer
./test.sh b L1-A-0-1

# test your parser
./test.sh d L1-A-0-1

# test with given test cases
ls "Tests (advanced)/inputs/" | xargs -n1 -i ./test.sh d {} > test.out
# or with -s (do not output the difference)
ls "Tests (advanced)/inputs/" | xargs -n1 -i ./test.sh -s d {} > test.out