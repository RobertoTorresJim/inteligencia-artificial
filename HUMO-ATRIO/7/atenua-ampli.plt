set terminal gif small size 640,480 xffffff x0000aa xc0c0c0 xff0000 x00ff00 x0000ff x000000 x00ffff xff00ff xffff00 xdda0dd x9500d3
set output "ateamp.gif"
set size 1.0, 1.0
set title "Laboratorio TAMDI, IA, 15I,  bloque: 18632"
set grid
set boxwidth 0.7
plot [0:63] "atenua.dat" using 1:2 title "atenua" w boxes fs solid 0.1 1, "ampli.dat" using 1:2 title "amplif" w boxes fs solid 0.1 2, "dc.dat" using 1:2 title "DC" w boxes fs solid 0.1 3, "pierde.dat" using 1:2 title "pierde" w boxes fs solid 0.1 4, "gana.dat" using 1:2 title "gana" w boxes fs solid 0.1 5 , "igual.dat" using 1:2 title "igual" w p 4 4, "signx.dat" using 1:2 title "signx" w p 1 3
