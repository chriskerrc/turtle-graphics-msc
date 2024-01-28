CC=gcc
COMMON= -Wall -Wextra -Wfloat-equal -Wpedantic -Wvla -std=c99 -Werror
DEBUG= -g3
SANITIZE= $(COMMON) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALGRIND= $(COMMON) $(DEBUG)
PRODUCTION= $(COMMON) -O3
VALGRINDFLGS= valgrind --leak-check=full

all: parse_s interp_s parse_v interp_v

parse_s: Parse/parse.h Parse/parse.c
	$(CC) Parse/parse.c -o parse_s -I./Parse $(SANITIZE) -lm

interp_s: Interp/interp.h Interp/interp.c Interp/neills_general.c neillsimplescreen.c neillsimplescreen.h
	$(CC) neillsimplescreen.c Interp/interp.c Interp/neills_general.c -o interp_s -I./Interp $(SANITIZE) -lm

parse_v: Parse/parse.h Parse/parse.c
	$(CC) Parse/parse.c -o parse_v -I./Parse $(VALGRIND) -lm

interp_v: Interp/interp.h Interp/interp.c Interp/neills_general.c neillsimplescreen.c neillsimplescreen.h
	$(CC) neillsimplescreen.c Interp/interp.c Interp/neills_general.c -o interp_v -I./Interp $(VALGRIND) -lm

valgrind: parse_v interp_v
	$(VALGRINDFLGS) ./parse_v TTLs/empty.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/empty.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/forward.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/ok_parse_fail_interp.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/set1.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/donothing.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/set2.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/turn.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/spiral.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/octagon1.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/octagon2.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/tunnel.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/labyrinth.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/hypno.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/5x5.ttl
	$(VALGRINDFLGS) ./parse_v TTLs/downarrow.ttl
	$(VALGRINDFLGS) ./interp_v TTLs/empty.ttl out_empty.txt
	$(VALGRINDFLGS) ./interp_v TTLs/empty.ttl out_empty.txt
	$(VALGRINDFLGS) ./interp_v TTLs/forward.ttl out_forward.txt
	$(VALGRINDFLGS) ./interp_v TTLs/set1.ttl out_set1.txt
	$(VALGRINDFLGS) ./interp_v TTLs/donothing.ttl out_donothing.txt
	$(VALGRINDFLGS) ./interp_v TTLs/fail_parse_ok_interp.ttl out_fail_parse_ok_interp.txt
	$(VALGRINDFLGS) ./interp_v TTLs/set2.ttl out_set2.txt
	$(VALGRINDFLGS) ./interp_v TTLs/turn.ttl out_turn.txt
	$(VALGRINDFLGS) ./interp_v TTLs/spiral.ttl out_spiral.txt
	$(VALGRINDFLGS) ./interp_v TTLs/octagon1.ttl out_octagon1.txt
	$(VALGRINDFLGS) ./interp_v TTLs/octagon2.ttl out_octagon2.txt
	$(VALGRINDFLGS) ./interp_v TTLs/tunnel.ttl out_tunnel.txt
	$(VALGRINDFLGS) ./interp_v TTLs/labyrinth.ttl out_labyrinth.txt
	$(VALGRINDFLGS) ./interp_v TTLs/hypno.ttl out_hypno.txt
	$(VALGRINDFLGS) ./interp_v TTLs/5x5.ttl out_5x5.txt
	$(VALGRINDFLGS) ./interp_v TTLs/downarrow.ttl out_downarrow.txt

run: all
	./parse_s TTLs/empty.ttl
	./parse_s TTLs/forward.ttl
	./parse_s TTLs/ok_parse_fail_interp.ttl
	./parse_s TTLs/set1.ttl
	./parse_s TTLs/donothing.ttl
	./parse_s TTLs/set2.ttl
	./parse_s TTLs/turn.ttl
	./parse_s TTLs/spiral.ttl
	./parse_s TTLs/octagon1.ttl
	./parse_s TTLs/octagon2.ttl
	./parse_s TTLs/tunnel.ttl
	./parse_s TTLs/labyrinth.ttl
	./parse_s TTLs/hypno.ttl
	./parse_s TTLs/5x5.ttl
	./parse_s TTLs/downarrow.ttl
	./interp_s TTLs/empty.ttl out_empty.txt
	./interp_s TTLs/forward.ttl out_forward.txt
	./interp_s TTLs/set1.ttl out_set1.txt
	./interp_s TTLs/donothing.ttl out_donothing.txt
	./interp_s TTLs/fail_parse_ok_interp.ttl out_fail_parse_ok_interp.txt
	./interp_s TTLs/set2.ttl out_set2.txt
	./interp_s TTLs/turn.ttl out_turn.txt
	./interp_s TTLs/spiral.ttl out_spiral.txt
	./interp_s TTLs/octagon1.ttl out_octagon1.txt
	./interp_s TTLs/octagon2.ttl out_octagon2.txt
	./interp_s TTLs/tunnel.ttl out_tunnel.txt
	./interp_s TTLs/labyrinth.ttl out_labyrinth.txt
	./interp_s TTLs/hypno.ttl out_hypno.txt
	./interp_s TTLs/5x5.ttl out_5x5.txt
	./interp_s TTLs/downarrow.ttl out_downarrow.txt
#	./interp_s TTLs/ok_parse_fail_interp.ttl out_ok_parse_fail_interp.txt

clean:
	rm -f parse_s interp_s parse_v interp_v out_*.txt  
