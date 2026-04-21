OBJECTS = main.o tokenizer.o errorhandler.o

jlox : $(OBJECTS)
	cc -o $@ $(OBJECTS)

.PHONY : run
run : jlox
	./jlox

.PHONY : clean
clean :
	rm jlox $(OBJECTS)
