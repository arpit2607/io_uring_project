#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <liburing.h>

#define FILE_PATH "testfile"
#define BUFFER_SIZE (8 * 1024 * 1024) // 8MB buffer
#define TOTAL_SIZE (1 * 1024 * 1024 * 1024) // 1GB total data

void perform_io_uring_write(int fd, struct io_uring *ring, char *buffer, unsigned nbytes) {
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    int ret;
    
    sqe = io_uring_get_sqe(ring);
    if (!sqe) {
        fprintf(stderr, "Could not get SQE for write.\n");
        return;
    }

    io_uring_prep_write(sqe, fd, buffer, nbytes, 0);

    ret = io_uring_submit(ring);
    if (ret < 0) {
        fprintf(stderr, "io_uring_submit for write: %s\n", strerror(-ret));
        return;
    }

    ret = io_uring_wait_cqe(ring, &cqe);
    if (ret < 0) {
        fprintf(stderr, "io_uring_wait_cqe for write: %s\n", strerror(-ret));
        return;
    }

    if (cqe->res < 0) {
        fprintf(stderr, "Async write failed: %s\n", strerror(-cqe->res));
    } else {
        //printf("Wrote %d bytes.\n", cqe->res);
    }

    io_uring_cqe_seen(ring, cqe);
}

void perform_io_uring_read(int fd, struct io_uring *ring, char *buffer, unsigned nbytes) {
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    int ret;

    sqe = io_uring_get_sqe(ring);
    if (!sqe) {
        fprintf(stderr, "Could not get SQE for read.\n");
        return;
    }

    io_uring_prep_read(sqe, fd, buffer, nbytes, 0);

    ret = io_uring_submit(ring);
    if (ret < 0) {
        fprintf(stderr, "io_uring_submit for read: %s\n", strerror(-ret));
        return;
    }

    ret = io_uring_wait_cqe(ring, &cqe);
    if (ret < 0) {
        fprintf(stderr, "io_uring_wait_cqe for read: %s\n", strerror(-ret));
        return;
    }

    if (cqe->res < 0) {
        fprintf(stderr, "Async read failed: %s\n", strerror(-cqe->res));
    } else {
        //printf("Read %d bytes: %.*s\n", cqe->res, cqe->res, buffer);
    }

    io_uring_cqe_seen(ring, cqe);
}

void perform_io_uring_operation(int sqpoll) {
    struct io_uring ring;
    struct io_uring_params params;
    int fd=-1, ret;
    char buffer[BUFFER_SIZE];
    memset(buffer, 'A', BUFFER_SIZE);

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

    // added a check after the open call to see if fd is -1, which indicates an error.
    // The perror function is used to print a generic error message.
    // The switch statement allows for more specific error handling based on the value of errno. This includes handling for common errors like permission issues (EACCES), file not found (ENOENT), and too many open files (EMFILE). You can add more cases to handle other specific error types as needed.
    // If an error occurs, the function prints an appropriate error message and returns early, preventing further operations on an invalid file descriptor.
    // Open file for read and write
    fd = open(FILE_PATH, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        // introduced a label cleanup_ring at the end of the function where the io_uring ring is cleaned up.
        // If an error occurs during file opening, the code jumps to the cleanup_ring label using goto. This ensures that the io_uring ring is properly cleaned up before exiting.
        // The close(fd) call is placed before the cleanup_ring label to ensure that the file descriptor is closed when the function completes normally.

        goto cleanup_ring;
        return;
    }

    fd = open(FILE_PATH, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        switch (errno) {
            case EACCES:
                fprintf(stderr, "Permission denied.\n");
                break;
            case ENOENT:
                fprintf(stderr, "File does not exist.\n");
                break;
            case EMFILE:
                fprintf(stderr, "Too many files open in system.\n");
                break;
            // You can add more cases as needed for different error types
            default:
                fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        }
        return;
    }

    // Perform chunked write operation
    for (int written = 0; written < TOTAL_SIZE; written += BUFFER_SIZE) {
        int chunk_size = (TOTAL_SIZE - written < BUFFER_SIZE) ? (TOTAL_SIZE - written) : BUFFER_SIZE;
        perform_io_uring_write(fd, &ring, buffer, chunk_size);
    }

    // Reset file offset for reading
    lseek(fd, 0, SEEK_SET);

    // Perform chunked read operation
    for (int read = 0; read < TOTAL_SIZE; read += BUFFER_SIZE) {
        int chunk_size = (TOTAL_SIZE - read < BUFFER_SIZE) ? (TOTAL_SIZE - read) : BUFFER_SIZE;
        perform_io_uring_read(fd, &ring, buffer, chunk_size);
    }
    close(fd);
cleanup_ring:
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
