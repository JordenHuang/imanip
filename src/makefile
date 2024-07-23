CC=gcc
CFALGS=-Wall -Wextra

target=amf.out gf.out sobel.out main_imanip.out

all: $(target)

amf.out: arithmetic_mean_filter.c
	$(CC) $(CFALGS) arithmetic_mean_filter.c -o amf.out -lm

gf.out: gaussian_filter.c
	$(CC) $(CFALGS) gaussian_filter.c -o gf.out -lm

sobel.out: sobel.c
	$(CC) $(CFALGS) sobel.c -o sobel.out -lm

main_imanip.out: main_imanip.c imanip.h
	$(CC) $(CFALGS) main_imanip.c -o main_imanip.out -lm

.PHONY: clean
clean:
	rm *.out
