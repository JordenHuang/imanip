CC=gcc
CFALGS=-Wall -Wextra

all: arithmetic_mean_filter

arithmetic_mean_filter: arithmetic_mean_filter.c
	$(CC) $(CFALGS) arithmetic_mean_filter.c -o arithmetic_mean_filter -lm
	./arithmetic_mean_filter

.PHONY: show
show:
	feh -F ./wallhaven-vqlr98.png ./out.png
