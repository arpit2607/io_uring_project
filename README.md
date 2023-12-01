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

# SPDK Setup and run
git clone https://github.com/spdk/spdk --recursive

sudo apt-get update

 sudo scripts/pkgdep.sh --all

./configure 
make

sudo scripts/setup.sh

come to root
git clone https://github.com/axboe/fio
cd fio
git checkout fio-3.29
./configure
make
sudo make install

come to root
cd spdk
./configure --with-fio=~/fio/ 
make

cd to io ring
sudo LD_PRELOAD=../spdk/build/fio/spdk_nvme fio spdk_non_sqpoll.fio