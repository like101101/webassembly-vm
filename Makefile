vm: vm.c
	gcc -g $< -o $@
  
clean:
	rm vm
