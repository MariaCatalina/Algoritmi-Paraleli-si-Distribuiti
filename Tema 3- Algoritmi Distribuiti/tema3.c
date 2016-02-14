#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define maxDimension 16
#define nrOfSolutions 10000

int dimMatrice, dimSudoku, dimProbl, rank;
int count = 0, newcount = 0;

/* vector pentru a retine cate solutii primeste un proces de la vecinii lui */
int *currentSolutionRecv;

/* matricea de sudoku citita din fisier */
int initialSudoku[maxDimension][maxDimension] = {{0}};

/* structura in care retin o solutie a unui proces */
typedef struct partialSolution{
  int processRank;
  int sudoku[maxDimension][maxDimension];
} solSudoku;

/* vector in care retin toate solutiile unui proces si solutiile combinate */
struct partialSolution *solutionsSudoku, *allCombinedSol;

void printTopologie(int matrice[maxDimension][maxDimension]){
  int i, j;
    for(i = 0; i < dimMatrice; i++){
      for (j = 0; j < dimMatrice;j ++)
        printf("%d ",matrice[i][j]);
      printf("\n");
    }
}

/* metoda pentru combinarea matricilor pentru stabilirea topologiei */
void combimMatrice(int matrice[maxDimension][maxDimension], int c[maxDimension][maxDimension]){
  int i = 0, j;
  for(i = 0; i < dimMatrice; i++){
    for(j = 0;j < dimMatrice; j++)
      if(matrice[i][j] == 0)
        matrice[i][j] = c[i][j];
  }
}

/* verifica daca matricea a numm si retureaza 1 in caz afirmativ */
int isMatNull(int matrice[maxDimension][maxDimension]){
  int i, j;
  for(i = 0; i< dimMatrice; i ++){
    for (j = 0; j < dimMatrice; j ++)
      if(matrice[i][j] != 0)
        return 0;
  }
  return 1;
}

/* metoda care actualizeaza topologie in functie de linia citita din fisier */
void completeazaTopologie(char *line, int topologie[maxDimension][maxDimension], int rank){
  char *p = (char *)malloc(255 * sizeof(char));
  int valoare;

  /* se trece peste prima valoare si se parseaza dupa '- */
  p = strtok(line,"-");
  p = strtok(NULL," ");

  /* se parseaza valorile */
  while(p){
    sscanf(p, "%d", &valoare);
    topologie[rank][valoare] = 1;
    p = strtok(NULL," ");
  }
  free(p);
}

/* metoda marcheaza patratele pe care fiecare proces o sa le calculeze
si extrage din matricea de sudoku initiala valorile corespunzatoare */
void macheazaPatrate(int matrice[dimSudoku][dimSudoku], int rank, int matSudoku[maxDimension][maxDimension], int sudoku[dimProbl][dimProbl]){
  int i, j, posI, posJ, nrPatrate = 0;

  for(i = 0; i < dimSudoku;i += dimProbl){
    for(j = 0; j < dimSudoku; j += dimProbl){
      if(nrPatrate == rank){
        posI = i;
        posJ = j;
      }
      nrPatrate++;
    }
  }
  
  int k = 0, p = 0;

  /* marchez patratele rank-ului si extrag matrcea de sudoku */
  for(i = posI; i < dimProbl + posI; i++){
    for(j = posJ; j < dimProbl + posJ; j ++){
      matrice[i][j] = rank;
      sudoku[k][p] = matSudoku[i][j];
      p ++;
    }
    k ++;
    p = 0;
  }
}

int UsedInRow(int sudoku[dimProbl][dimProbl], int row, int num){
  int i;
  for(i = 0; i < dimProbl; i++)
    if(sudoku[row][i] == num)
      return 1;
  return 0;
}

int UsedInCol(int sudoku[dimProbl][dimProbl], int col, int num){
  int i;
  for(i = 0; i < dimProbl; i++)
    if(sudoku[i][col] == num)
      return 1;
  return 0;
}

int UsedInBox(int sudoku[dimProbl][dimProbl], int num){
  int i, j;
  for(i = 0; i < dimProbl; i++)
    for(j = 0; j < dimProbl; j++)
      if(sudoku[i][j] == num)
        return 1;
  return 0;
}

/* verific daca valoare curenta din sudoku se gaseste in datele initiale
pentru a micsora numarul de solutii invalide */
int UsedInInitialSudoku(int row, int col, int num){
  int i, j, posI = 0, posJ = 0, nrPatrate = 0;


  /* calculare pozitii in matricea initala in funtie de rank */
  for(i = 0; i < dimSudoku; i += dimProbl){
    for(j = 0; j < dimSudoku; j += dimProbl){
      if(nrPatrate == rank){
        posI = i;
        posJ = j;
      }
      nrPatrate++;
    }
  }

  /* calculare pozitii noi */
  row += posI;
  col += posJ;

  /* verificare linie */
  for(i = 0; i < maxDimension; i++)
    if(initialSudoku[row][i] == num){
      return 1;
    }

  /* verificare coloana */
  for(i = 0; i < maxDimension; i++)
    if(initialSudoku[i][col] == num){
      return 1;
    }

  return 0;
}

int isSafe(int sudoku[dimProbl][dimProbl], int row, int col, int num){
   return !UsedInRow(sudoku, row, num) &&
           !UsedInCol(sudoku, col, num) &&
           !UsedInBox(sudoku, num) &&
           !UsedInInitialSudoku(row, col, num);
}

/* metoda pentru adaugarea unei solutie sudoku in vector pe pozitia corespunzatoare procesului */
void addSolution(int sudoku[dimProbl][dimProbl]){
  int i, j, posI, posJ, nrPatrate = 0;

  struct partialSolution s;
  s.processRank = rank;
  
  /* initializez toata matricea */
  for(i = 0; i < maxDimension; i++)
    for(j = 0; j < maxDimension; j++){
      s.sudoku[i][j] = 0;
  }

  /* adaug valori doar in patratul corespunzator */
  for(i = 0; i < dimSudoku;i += dimProbl){
    for(j = 0; j < dimSudoku; j += dimProbl){
      if(nrPatrate == rank){
        posI = i;
        posJ = j;
      }
      nrPatrate++;
    }
  }
  
  int k = 0, p = 0;
  
  for(i = posI; i < dimProbl + posI; i++){
    for(j = posJ; j < dimProbl + posJ; j ++){
      s.sudoku[i][j] = sudoku[k][p];
      p ++;
    }
    k ++;
    p = 0;
  }
   
  solutionsSudoku[count] = s;
  count ++;
}

/* metoda pentru generarea solutiilor prin backtracking */
void solve(int sudoku[dimProbl][dimProbl], int row, int col, int value){
  int i;

  if(!isSafe(sudoku, row, col, value))
    return;

  sudoku[row][col] = value;
  
  do{
    col ++;

    if(col >= dimProbl){
      col = 0;
      row ++;
      
      if(row == dimProbl){
        /* am gasit o solutie pentru sudoku si o adaug in vectorul de solutii */
        addSolution(sudoku); 
        return;
      }
    }
  } while(sudoku[row][col] != 0);

  for(i = 1; i <= dimProbl * dimProbl; i++)
    solve(sudoku, row, col, i);
 
  sudoku[row][col] = 0;
}

void SolveSudoku(int sudoku[dimProbl][dimProbl]){
    int row, col, i;
    row = 0;
    col = -1;

    do {
      col ++;
    
      if( col >= dimProbl){
        col = 0;
        row ++;
      }
    }while(sudoku[row][col] != 0);

    for(i = 1; i <= dimProbl * dimProbl; i++)
      solve(sudoku, row, col, i);
}

void copyMatrix(int a[maxDimension][maxDimension],int b[maxDimension][maxDimension]){
  int i, j;
  for(i = 0; i < maxDimension; i ++)
    for(j = 0; j < maxDimension; j ++)
      a[i][j] = b[i][j];
}

/* metoda verifica daca sudoku e complet */
int isCompleteSudoku(int sudoku[maxDimension][maxDimension]){
  int i, j;
  for(i = 0; i< dimSudoku; i++){
    for(j = 0; j< dimSudoku; j++){
      if(sudoku[i][j] == 0)
        return 0;
    }
  }
  return 1;
}

int isGoodValue(int sudokuComplet[maxDimension][maxDimension], int row, int col, int num){
  int i;

  /* verific linia */
  for(i = 0; i < maxDimension; i ++)
    if(sudokuComplet[row][i] == num)
      return 0;

  /* verific coloana */
  for(i = 0; i < maxDimension; i ++)
    if(sudokuComplet[i][col] == num)
      return 0; 

  return 1;
}

/* metoda verifica daca 2 matrici de sudoku se pot combina */
int verifySolution(int sudokuCurent[maxDimension][maxDimension], int sudokuPrimire[maxDimension][maxDimension]){
  int i, j, num;
  for(i = 0; i < dimSudoku; i ++){
    for(j = 0; j < dimSudoku; j ++){
      if(sudokuPrimire[i][j] != 0){
        num = sudokuPrimire[i][j];
        
        if(isGoodValue(sudokuCurent, i, j, num))
          sudokuCurent[i][j] = num;
        else
          return 0;
      }
    }
  }
  return 1;
}

/* metoda combina o solutie cu toate solutiile copiilor */
void combine(int sudoku[maxDimension][maxDimension], struct partialSolution **childrenSolutions,int parinte, int vecin){
  int i;
  struct partialSolution ps;
  int copy[maxDimension][maxDimension] = {{0}};
  int pastreazaSudoku[maxDimension][maxDimension] = {{0}};

  if(vecin > dimMatrice - 1){
    /* salvez solutia gasita */
    copyMatrix(ps.sudoku, sudoku);
    allCombinedSol[newcount] = ps;
    newcount ++;
  }
  else{
     /* daca am primit solutii de la vecin le prelucrez */
     if(currentSolutionRecv[vecin] > 0){ 
     
      /* parcurg lista cu vecini pana gasesc o combinatie buna */
      for(i = 0; i < currentSolutionRecv[vecin]; i++){
        ps = childrenSolutions[vecin][i];
        copyMatrix(copy, sudoku);
        copyMatrix(pastreazaSudoku, sudoku);

        if(verifySolution(copy, ps.sudoku)){
          copyMatrix(sudoku, copy);
          
          combine(sudoku, childrenSolutions, parinte, vecin + 1);
        } 
        /* reset la vechea sudoku */
        copyMatrix(sudoku, pastreazaSudoku);
      }
    }
    /* apelez pentru urmatorul vecin */
    else{
        combine(sudoku, childrenSolutions, parinte, vecin + 1);
    }    
  }
}

/* metoda pentru combinare tuturor solutiilor si trimiterea lor la parinte */
void combineAllSolutions(struct partialSolution **childrenSolutions, int parinte){
  int i, vecin, tagCount = 3, tag = 2;
  struct partialSolution ps;
  int copy[maxDimension][maxDimension] = {{0}};

  /* fiecare solutia a procesului curent incerc sa o combim cu toate solutiile de la copii */
  for(i = 0; i < count; i ++){
    ps = solutionsSudoku[i];
    copyMatrix(copy, ps.sudoku);

    vecin = 0;
    /* se combina solutie cureta cu toate solutiile copiilor */
    combine(copy, childrenSolutions, parinte, vecin); 
  }

  /* trimit solutiile parintilor */
  if(rank != 0){
    MPI_Send(&newcount, 1, MPI_INT, parinte, tagCount, MPI_COMM_WORLD);

    for(i = 0; i < newcount; i ++){
      ps = allCombinedSol[i];

      copyMatrix(copy, ps.sudoku);
      MPI_Send(copy, maxDimension * maxDimension, MPI_INT, parinte, tag, MPI_COMM_WORLD);
    }
  }
}

/* metoda care calculeaza next hop */
void getNextHop(int topologie[maxDimension][maxDimension], int process, int parinte){
  int i, flag = 0;
 
  /* daca in matrice linia e 0 => nu am legatura catre nodul acela */
  for(i = 0; i < dimMatrice; i++){
    if(topologie[process][i] != 0){
      flag = 1;
    }
  }

  if(flag == 0){
    printf("%d \n", parinte);
    return;
  }

  else{
    /* parcurg recursiv toate legeturile nodului curent pana ajung la rang */
    for(i = 0; i < dimMatrice; i ++){
      /* daca am ajuns la o rank-ul curent am gasit next hop */
      if(topologie[process][i] == 1 && i == rank){
        printf("%d\n",process);
        return;
      }
      else 
        /* aplez recursiv pentru totii vecinii */
        if(topologie[process][i] == 1 && i != rank){
          getNextHop(topologie, i, parinte);
        }
    }
  }

}

int main(int argc, char *argv[])  {
  int numtasks;  
  MPI_Status Stat;
 
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  dimMatrice = numtasks;

  int parinte = -1, receiveProces, i, j, tag = 1, nrVecini;
  int matPrimesc[maxDimension][maxDimension] = {{0}};
  int matNull[maxDimension][maxDimension] = {{0}};
  int topologie[maxDimension][maxDimension] = {{0}};
 
  /* citire din fisier */
  FILE *fTopologie = fopen(argv[1], "r");

  char *line =  (char *)malloc(255 * sizeof(char));
  char *copy =  (char *)malloc(255 * sizeof(char));

  int lineProcess;

  /* citesc fiecare linie si o parsez, daca primul numar corespunde cu 
  nr procesului meu o parsez in continuare */ 
  while( fgets(line,1024, fTopologie)){

    /* verific ce valoare are linia curent citita */
    strcpy(copy, line);
    sscanf(line, "%d", &lineProcess);
   
    /* daca este egala cu rank-ul se completeaza topologia */
    if(lineProcess == rank){
      completeazaTopologie(line, topologie, rank);
    }
  }

  /* asteapta mesaje de la parinti */   
  if(rank != 0){
    MPI_Recv(matPrimesc, maxDimension * maxDimension, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Stat);
    parinte = Stat.MPI_SOURCE;
  }

  /* tag = 1 sonda, tag = 0 ecou */

  /* trimite catre toti copii */
  nrVecini = 0;
  for(i = 0; i < dimMatrice; i ++){
      if( topologie[rank][i] == 1 && i != parinte){
        tag = 1;
        MPI_Send(matNull, maxDimension * maxDimension, MPI_INT, i, tag, MPI_COMM_WORLD);
        nrVecini ++;
      }
  }
  int tagNou;

  while(nrVecini > 0){
   
    MPI_Recv(matPrimesc, maxDimension * maxDimension, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Stat);
    receiveProces = Stat.MPI_SOURCE;
    tagNou = Stat.MPI_TAG;

    if(tagNou == 1){
      tag = 0;
      MPI_Send(matNull, maxDimension * maxDimension, MPI_INT, receiveProces, tag, MPI_COMM_WORLD);
    }
    else 
      if(tagNou == 0){

        if(!isMatNull(matPrimesc)){
           combimMatrice(topologie, matPrimesc);
        }
        else{ 
          /* sterg legatura dintre ciclu */
          topologie[rank][receiveProces] = 0;
        }
        nrVecini --;
     }
  }

  if(rank != 0){
    tag = 0;
    MPI_Send(topologie, maxDimension * maxDimension, MPI_INT, parinte, tag, MPI_COMM_WORLD);
  }

  /* se pune basiera pentru ca toate procesele sa termine algoritmul
  de stabilire a topologiei */
  MPI_Barrier(MPI_COMM_WORLD);

  /* afisare topologie proces 0 */
  if(rank == 0){
    printf("Topologia nodului 0\n");
    printTopologie(topologie);
  }

  /* deschid fisierul de intrare pentru sudoku si citesc matricea */
  FILE *fSudoku = fopen(argv[2], "r");

  int tag2 = 2, tagCount = 3;
  
  /* tag2 == 2 pentru solutii trimise
     tagCount == 3 pentru numarul de solutii */

  fscanf(fSudoku,"%d",&dimProbl);

  /* modific dimensiunea problemei pentru a aloca matricea */
  dimSudoku = dimProbl * dimProbl;

  /* citire din fiseier matricea de sudoku initiala */
  for(i = 0; i < dimSudoku; i ++)
    for(j = 0; j < dimSudoku; j ++)
      fscanf(fSudoku,"%d",&initialSudoku[i][j]);

  solutionsSudoku = (struct partialSolution*)malloc(nrOfSolutions * sizeof(struct partialSolution));
  allCombinedSol = (struct partialSolution*)malloc(nrOfSolutions * sizeof(struct partialSolution));

  /* alocare matrice ce va fi rezolvata 
  e -1 la inceput si fiecare proces isi va marca patratele 
  prin nr procesului */
  int matSudokuMarcare[dimSudoku][dimSudoku];
 
  /* matricea in care se extrage sudoku pentru fiecare proces */
  int sudoku[dimProbl][dimProbl];

  int sudokuPrimire[maxDimension][maxDimension];
  int copySudoku[maxDimension][maxDimension];

  /* marchez patratele procesului curent */
  macheazaPatrate(matSudokuMarcare, rank, initialSudoku, sudoku);

  /* calculez numarul de vecini */
  nrVecini = 0;
  for(i = 0; i < dimMatrice; i ++){
    if(topologie[rank][i] == 1 && i != parinte)
      nrVecini ++;
  }

  /* apelez functia pentru calcularea tuturor solutiilor */
  SolveSudoku(sudoku);

  struct partialSolution ps;  
  int noSolutionGeneral = 0, nrCount = 0, pos = 0;
  
  currentSolutionRecv = (int *)malloc(dimMatrice * sizeof(int));
  /* initializare */
  for(i = 0; i < dimMatrice; i++){
    currentSolutionRecv[i] = 0;
  }

  /* creez o lista de solitii pentru fiecare copil */ 
  struct partialSolution **allpartialSolutions;
  
  /* alocare dimensiune pentru a retine solutiile toturor solutiilor */
  allpartialSolutions = (struct partialSolution**)malloc(maxDimension * sizeof(struct partialSolution*));
  
  for(i = 0; i < dimMatrice; i ++){
    allpartialSolutions[i] = (struct partialSolution*)malloc(nrOfSolutions * sizeof(struct partialSolution));
  }

  /* daca nr vecini e zero => sunt frunza */
  if(nrVecini == 0){
    /* trimit mai intai paritelui numarul de solutii */
    MPI_Send(&count, 1, MPI_INT, parinte, tagCount, MPI_COMM_WORLD);
   
    /* trimit pe rand toate solutiile gasite */ 
    for(i = 0; i < count; i ++){
      ps = solutionsSudoku[i];
      copyMatrix(copySudoku, ps.sudoku);
      MPI_Send(copySudoku, maxDimension * maxDimension , MPI_INT, parinte, tag2, MPI_COMM_WORLD);
    }
  }
  else{
      /* astept numarul de solitii de la fiecare copil */
      for(i = 0; i < dimMatrice; i ++){
        if(topologie[rank][i] == 1 && i != parinte){
          MPI_Recv(&nrCount, 1, MPI_INT, i, tagCount, MPI_COMM_WORLD, &Stat);
          noSolutionGeneral += nrCount;
        }
      }

      /* astept fiecare solutie de la copii si le salvez in vector */
      while(noSolutionGeneral > 0){
        MPI_Recv(sudokuPrimire, maxDimension * maxDimension, MPI_INT, MPI_ANY_SOURCE, tag2, MPI_COMM_WORLD, &Stat);
        receiveProces = Stat.MPI_SOURCE;
        
        /* adaug in vector solutia, corespunzatoare procesului primit */
        pos = currentSolutionRecv[receiveProces];
        
        /* construiesc structura */
        ps.processRank = receiveProces;
        for(i = 0; i < maxDimension; i++){
          for(j = 0; j< maxDimension; j++)
            ps.sudoku[i][j] = sudokuPrimire[i][j];
        }
        
        allpartialSolutions[receiveProces][pos] = ps;

        currentSolutionRecv[receiveProces] = ++ pos;
        noSolutionGeneral --;  
      }

      combineAllSolutions(allpartialSolutions, parinte);
  }
  /* afisare in fisier */
  if(rank == 0) {
    FILE* fRezultat = fopen(argv[3], "w");
    fprintf(fRezultat, "%d\n", dimProbl);

    if(newcount == 0) {
      fprintf(fRezultat, "Fara solutie!\n");
    }
    else {
      ps = allCombinedSol[0];
      /* afiseaza prima solutie finala gasita */
      for(i = 0; i < dimSudoku; i++) {
        for(j = 0; j < dimSudoku; j++) {
          fprintf(fRezultat, "%d ", ps.sudoku[i][j]);
        }
        fprintf(fRezultat, "\n");
      }
    }
    fclose(fRezultat);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  usleep(500 * rank);

  printf("Tabela de rutare pentru procesul %d\n",rank );
  for(i = 0; i < dimMatrice; i ++){
    if(i != rank){
      printf("%d -> next hop ", i);
      getNextHop(topologie, i, parinte);
    }
  }
  
  fclose(fTopologie);
  fclose(fSudoku);

  for(i = 0; i < dimMatrice; i++){
    free(allpartialSolutions[i]);
  }

  free(allpartialSolutions);
  free(solutionsSudoku);
  free(allCombinedSol);
  free(currentSolutionRecv);
  free(line);
  free(copy);
  MPI_Finalize();
  return 0;
}