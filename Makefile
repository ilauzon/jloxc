OBJECTS = scanning.o tokenizer.o

jlox : $(OBJECTS)
	cc -o $@ $(OBJECTS)

.PHONY : run
run : jlox
	./jlox

.PHONY : clean
clean :
	rm jlox $(OBJECTS)
