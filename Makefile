
%.o: %.c
	gcc -o $@ -c $< -g3 -O2 -Wall

otfprint: otfprint.o otf_mem.o otf_cff.o otf_vm.o otf_sid.o otf_kern.o otf_cmap.o otf_gpos.o
	gcc -o $@ $^

clean::
	rm -f *~ otfprint.o otf_mem.o otf_cff.o otf_vm.o otf_sid.o otf_kern.o otf_cmap.o otf_gpos.o otfprint
