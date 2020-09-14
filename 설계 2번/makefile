ssu_shell : ssu_shell.c ssu_shell.h pps
	gcc ssu_shell.c -o ssu_shell

pps : ssu_ps.c ssu_ps.h
	gcc ssu_ps.c -o pps

clean : 
	rm -f ssu_shell
	rm -f pps
