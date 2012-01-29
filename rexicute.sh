mpirun -np $1 ./mandelbrot -h $2 -w $3
R --no-save << EOT
A = scan(file="output.csv",sep=",")
dim(A) = c($3,$2)
image(A)
quit("no")
EOT
evince "./Rplots.pdf"
