CFLAGS = -O3 -ansi -pedantic -pipe -Wall -Wextra -Werror 
LDFLAGS = -lX11 -lXrandr

.PHONY: all clean

all:
	$(CC) $(CFLAGS) -o onscreenresize onscreenresize.c $(LDFLAGS) 

clean:
	rm -f onscreenresize
