# io_uring_project

sudo apt-get update

sudo apt-get install build-essential libaio-dev liburing-dev fio


# fio installation

sudo apt-get install git build-essential zlib1g-dev libaio-dev

git clone https://github.com/axboe/fio.git

cd fio

./configure

make

sudo make install

# compiling and running the program

gcc -o io_uring_test io_uring_test.c -luring

./io_uring_test 0  # Non-SQPOLL

./io_uring_test 1  # SQPOLL

# running fio benchmarks

fio non_sqpoll.fio

fio sqpoll.fio

# perf

sudo apt-get install linux-tools-5.15.0-86-generic

# Basic perf usage

perf stat ./io_uring_test

# Advanced perf usage

perf record -g ./io_uring_test

perf report

perf stat -e cache-misses,cache-references,instructions,cycles ./io_uring_test

(If perf doesn't work) - echo 1 | sudo tee /proc/sys/kernel/perf_event_paranoid 
