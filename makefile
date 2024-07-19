CC=gcc
CFALGS=-Wall -Wextra

all: amf

amf: arithmetic_mean_filter.c
	$(CC) $(CFALGS) arithmetic_mean_filter.c -o amf -lm
	./amf

.PHONY: show
show:
	feh -F ./wallhaven-vqlr98.png ./arithmetic_mean_filter.png
