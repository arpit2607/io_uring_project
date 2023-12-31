# io_uring_project

sudo apt-get update

sudo apt-get install build-essential libaio-dev liburing-dev fio

# compiling and running the program

gcc -o io_uring_test io_uring_test.c -luring

./io_uring_test 0  # Non-SQPOLL

./io_uring_test 1  # SQPOLL

# running fio benchmarks
dd if=/dev/zero of=~/testfile bs=1G count=1

fio non_sqpoll.fio

fio sqpoll.fio

# perf

sudo apt-get install linux-tools-5.15.0-86-generic

# Basic perf usage

sudo perf record -a -g -o perf_with_sqpoll.data -- fio sqpoll.fio

sudo perf record -a -g -o perf_without_sqpoll.data -- fio non_sqpoll.fio

sudo perf report -i perf_with_sqpoll.data > report_with_sqpoll.txt

sudo perf report -i perf_without_sqpoll.data > report_without_sqpoll.txt

diff report_with_sqpoll.txt report_without_sqpoll.txt

vimdiff report_with_sqpoll.txt report_without_sqpoll.txt

# Advanced perf usage

perf record -g ./io_uring_test

perf report

perf stat -e cache-misses,cache-references,instructions,cycles ./io_uring_test

(If perf doesn't work) - echo 1 | sudo tee /proc/sys/kernel/perf_event_paranoid 
