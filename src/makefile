CC=gcc
CFALGS=-Wall -Wextra

target=amf.out gf.out sobel.out

all: $(target)

amf.out: arithmetic_mean_filter.c
	$(CC) $(CFALGS) arithmetic_mean_filter.c -o amf.out -lm
	# ./amf.out

gf.out: gaussian_filter.c
	$(CC) $(CFALGS) gaussian_filter.c -o gf.out -lm
	# ./gf.out

sobel.out: sobel.c
	$(CC) $(CFALGS) sobel.c -o sobel.out -lm

.PHONY: clean
clean:
	rm *.out
