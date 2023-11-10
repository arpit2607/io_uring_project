#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <liburing.h>

#define FILE_PATH "testfile"
#define BUFFER_SIZE 1024

void perform_io_uring_operation(int sqpoll) {
    struct io_uring ring;
    struct io_uring_params params;
    int fd, ret;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    char buffer[BUFFER_SIZE] = "Hello, io_uring!";

    if (sqpoll) {
        printf("SQPOLL\n");
        memset(&params, 0, sizeof(params));
        params.flags = IORING_SETUP_SQPOLL;
        ret = io_uring_queue_init_params(32, &ring, &params);
    } else {
        printf("NOSQPOLL\n");
        ret = io_uring_queue_init(32, &ring, 0);
    }
    
    if (ret < 0) {
        perror("io_uring_queue_init");
        return;
    }

    fd = open(FILE_PATH, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return;
    }

    sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        fprintf(stderr, "Could not get SQE.\n");
        return;
    }

    io_uring_prep_write(sqe, fd, buffer, BUFFER_SIZE, 0);

    ret = io_uring_submit(&ring);
    if (ret < 0) {
        fprintf(stderr, "io_uring_submit: %s\n", strerror(-ret));
        return;
    }

    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
        fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-ret));
        return;
    }

    if (cqe->res < 0) {
        fprintf(stderr, "Async write failed: %s\n", strerror(-cqe->res));
    } else {
        printf("Wrote %d bytes.\n", cqe->res);
    }

    io_uring_cqe_seen(&ring, cqe);

    close(fd);
    io_uring_queue_exit(&ring);
}

int main(int argc, char *argv[]) {
    int use_sqpoll = 0;

    if (argc > 1) {
        use_sqpoll = atoi(argv[1]);
    }

    perform_io_uring_operation(use_sqpoll);

    return 0;
}
