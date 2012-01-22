# Makefile for the MPI Hello World project
# CIS 455/555
# updated Jan 2012 for OpenMPI

CXX = mpic++

mandelbrot:	mandelbrot.o
	${LINK.cc} -o mandelbrot mandelbrot.o

mandelbrot.o:	mandelbrot.c
	${LINK.cc} -c mandelbrot.c
.PHONY:	clean

clean:
	-/bin/rm -f *.o *~ core

