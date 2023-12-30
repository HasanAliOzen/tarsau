CC = gcc
CFLAGS = -Wall -Werror

tarsau: tarsau.c
    $(CC) $(CFLAGS) -o tarsau tarsau.c

clean:
    rm -f tarsau