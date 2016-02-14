Popa Maria-Catalina 331CC

	Tema 3 - APD Sudoku

	Versiunea de module: libraries/openmpi-1.6-gcc-4.7.0

	Stabilirea topoligiei a fost realizata conform algorimului din curs, iar
in momentul cand se primeste o topologie goala se sterge legatura cu acel nod
pentru a elimina ciclurile.
	Fiecare proces va citi din fisierul de intrare intreaga matrice de sudoku 
si o va retine in varialbila initialSudoku. 
	Structura partialSolution contine rank-ul procesului si matricea de sudoku
de dimensiune 16 x 16, din care se va folosi cat este nevoie in functie de
dimensiunea problemei. Variabile solutionsSudoku si allcombinedSol contin
solutiile procesului curent, respectiv toate solutiile primite de la copii.
Din matricea initiala se extrage patratul corespunzator conventiei:
| proces 0 | proces 1|
| proces 2 | proces 3|  (pentru dimensiune 2 x 2 pentru fiecare proces)

	Generarea solutiilor pentru fiecare matrice de sudoku se face folosind
backtracking. Verificare valorilor se face conform regulilor de sudoku dar se 
verifica liniile si coloanele in matricea de sudoku initiala pentru a reduce numarul
solutiilor incorecte.
	Daca procesul curent este frunza acesta trimite la parinte numarul lui de 
solutii si apoi fiecare solutie, pe rand. Daca procesul nu este frunza el
asteapta de la totii copii numarul de solutii si rezolvarile lor pe care le
salveaza in variabila allpartialSolutions. In vectorul crrentSolutionsRecv
se retine pentru fiecare proces numarul de soltuii ale copiilor sau 0 senmificand
ca nu exista legatura cu acel nod.
	Metoda combineAllSolutions parcurge fiecare solutie proprie si apeleaza metoda combine.
Aceasta parcurge in mod recursiv toate solutiile copiilor le combina si le salveaza.
Dupa ce se termina aceasta parcurgere procesul != 0 trimite numarul de solutii
si solutiile la parinte. Daca procesul == 0 se afiseaza in fisier prima solutie
gasita.

	Pentru afisarea topologiei, prin next hop, fiecare proces parcurge lista
cu toate procesele si apleaza functia getNextHop. Acesta verifica daca in 
topologia primita nu exista nici o intrare pentru procesul cerut inseamana ca
next hop este parinte. Altfel rezulta ca nodul de afla in subarbore. Se 
parcurg recursiv nodurile de la procesul cerut pana cand se ajunge la rank-ul
propriu si se afiseaza rezultatele. Pentru a nu se intersecata valorile la 
afisare am folosit  MPI_Barrier(MPI_COMM_WORLD) si usleep(500 * rank).