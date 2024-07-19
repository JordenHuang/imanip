CC=gcc
CFALGS=-Wall -Wextra

target=amf.out

all: $(target)

amf.out: arithmetic_mean_filter.c
	$(CC) $(CFALGS) arithmetic_mean_filter.c -o amf.out -lm
	./amf.out

.PHONY: show clean
show:
	feh -F ./wallhaven-vqlr98.png ./arithmetic_mean_filter.png

clean:
	rm *.out
