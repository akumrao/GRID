

# the target is obtained linking all .o files
all: make5 make6


make5:
	make -f Makefile5

make6:
	make -f Makefile6	

purge: clean

clean:
	make -f make -f Makefile5 purge && make -f Makefile6 purge

###############################################################################
################################################################################
