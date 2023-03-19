#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>


const int buf_size = 5000;

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("Error: main in out\n");
        exit(0);
    }
    int in;
    int out;
    int fifo_desc;
    int result;
    char* pipe_r = "prc.fifo";
    char* pipe_w = "pcw.fifo";
    char buffer[buf_size];
    ssize_t read_b;
    ssize_t written_b;

    mknod(pipe_r, S_IFIFO | 0666, 0);
    mknod(pipe_w, S_IFIFO | 0666, 0);

    result = fork();
    if (result < 0) {
        printf("Error fork child\n");
        exit(-1);
    } else if (result > 0) {
        if((fifo_desc = open(pipe_r, O_WRONLY)) < 0){
            printf("[R]: Error open FIFO for writting\n");
            exit(-1);
        }
        printf("[R]: Reading from file %s...\n", argv[1]);
        in = open(argv[1], O_RDONLY);
        if (in < 0) {
            printf("[R]: Error open file\n");
            exit(1);
        }
        read_b = read(in, buffer, buf_size);
        if (read_b > 0) {
            printf("[R]: Writing to pipe %ld bytes\n", read_b);
            written_b = write(fifo_desc, buffer, read_b);
            if (written_b != read_b) {
                printf("[R]: Error write all string to pipe\n");
                exit(-1);
            }
        }
        close(in);
        close(fifo_desc);
        printf("[R]: Ended job\n");
    } else {
        result = fork();
        if (result < 0) {
            printf("Error fork child\n");
            exit(-1);
        } else if (result > 0) {
            out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
            if (out < 0) {
                printf("[W]: Error create file\n");
                exit(1);
            }
            if((fifo_desc = open(pipe_w, O_RDONLY)) < 0){
                printf("[W]: Error open FIFO for reading\n");
                exit(-1);
            }
            printf("[W]: Reading from pipe...\n");
            read_b = read(fifo_desc, buffer, buf_size);
            printf("[W]: Writing to file %s %ld bytes\n", argv[2], read_b);
            written_b = write(out, buffer, read_b);
            close(out);
            close(fifo_desc);
            printf("[W]: Ended job\n");
        } else {
            if((fifo_desc = open(pipe_r, O_RDONLY)) < 0){
                printf("[PROCESS]: Error open FIFO for reading\n");
                exit(-1);
            }
            read_b = read(fifo_desc, buffer, buf_size);
            printf("[PROCESS]: Processing string of %ld bytes...\n", read_b);
            for (int i = 0; i < read_b / 2; i++) {
                char tmp = buffer[i];
                buffer[i] = buffer[read_b - i - 1];
                buffer[read_b - i - 1] = tmp;
            }
            close(fifo_desc);
            if((fifo_desc = open(pipe_w, O_WRONLY)) < 0){
                printf("[PROCESS]: Error open FIFO for writting\n");
                exit(-1);
            }
            printf("[PROCESS]: Writing to pipe %ld bytes\n", read_b);
            written_b = write(fifo_desc, buffer, read_b);
            close(fifo_desc);
            printf("[PROCESS]: Ended job\n");
        }
    }
    return 0;
}