set terminal gif small size 640,480 xffffff x0000aa xc0c0c0 xff0000 x00ff00 x0000ff xcdb5cd xadd8e6 x0000ff xdda0dd x9500d3
set output "fig.gif"
set size 1.0, 1.0
set title "Laboratorio TAMDI, IA, 15I, Y bloque: 19115"
set grid
plot "blkp1.dat" using 1:2 title "pic1" w l 1, "blkp2.dat" using 1:2 title "pic2" w l 3
