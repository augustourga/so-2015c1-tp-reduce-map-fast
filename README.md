# tp-2015-1c-milanesa

#########################################
# INSTALACION BERKELEY DB
Primero hay que buscar y bajarse las bibliotecas de Berkeley. Esto se hace ejecutando el comando por consola:

//Este comando te dice que paquetes hay para instalar con un nombre que tenga "libdb5" y te los muestra abajo
apt-cache search libdb5		

...
libdb5.3 - Berkeley v5.3 Database Libraries [runtime]
libdb5.3-dbg - Berkeley v5.3 Database Libraries [debug]
libdb5.3-dev - Berkeley v5.3 Database Libraries [development]
...

Entonces los instalamos con:
sudo apt-get install libdb5.3 libdb5.3-dbg libdb5.3-dev

Por ultimo, desde eclipse hay que linkear el proyecto que tengo con esta nueva biblioteca
Para esto:
En el Project Explorer (La columna de la izquierda), click derecho en tu proyecto que queres agregar la biblioteca > Properties > C/C++ Build > Settings > Tool Settings.
Hay vas a ver  G++ Linker > Libraries > Libraries (notar como dice -l, que es lo que estamos buscando).
Click en el boton de + y pones el nombre de la biblioteca, en este caso "db" (sin el -l).

##########################################