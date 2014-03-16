
%.o: %.c
	gcc -o $@ -c $< -g -O2 -Wall

otf: otfread.o otf_mem.o otf_cff.o otf_vm.o otf_kern.o otf_cmap.o
	gcc -o $@ $^

clean::
	rm -f *~ otfread.o otf_mem.o otf_cff.o otf_vm.o otf_kern.o otf_cmap.o otf
