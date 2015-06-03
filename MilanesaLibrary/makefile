UTILES=libutiles.so
LIB_DIR=/usr/lib
INCLUDE_DIR=/usr/include
LIBS= -lcommons
FLAGS= -shared -fPIC -g -Wall

C_SRCS = $(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS = $(shell find . -iname "*.h" | tr '\n' ' ')

compile:
	gcc $(FLAGS) $(C_SRCS) $(H_SRCS) -o $(UTILES) $(LIBS)

install: compile
	cp -u $(UTILES) $(LIB_DIR)
	cp --parents -u $(H_SRCS) $(INCLUDE_DIR)

uninstall: clean
	rm -f $(LIB_DIR)/$(UTILES)
	rm -rf $(INCLUDE_DIR)/utiles 
        
clean:
	rm -f $(UTILES)

# PHONY targets son los que no producen un archivo llamado como el target
# Se los marca para evitar confusion si se crea un archivo llamado asi
.PHONY: compile install uninstall clean
