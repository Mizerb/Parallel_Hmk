/***************************************************************************/
/* Template for Asssignment 1 **********************************************/
/* Your Name Here             **********************************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
// New Includes for Assignment 2
#include<clcg4.h>
#include<mpi.h>


/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/
#define ALIVE 3
#define TO_BE_ALIVE 1 
#define DEAD  0
#define TO_BE_DEAD 2

#define INITAL_RNG_GAIN 0.2
#define LINED 0

#define MASTER 0


/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/



// Bring over from your code
unsigned char **g_GOL_CELL=NULL;

unsigned int g_x_cell_size=0;
unsigned int g_y_cell_size=0;

unsigned int g_num_ticks=0;

double g_thresh_hold=0.0;

int g_mpi_myrank;

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// Bring over from your code
void allocate_and_init_cells();
void compute_one_tick();
void output_final_cell_state();
void print_cells(FILE* stream); //general thing for error seaching


void calculate_tick();
void execute_tick();
unsigned char look_around( int x , int y);

unsigned int RNG_check(double gain);

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
//    int i = 0;
    int mpi_myrank;
    int mpi_commsize;
    int i;
// Example MPI startup and using CLCG4 RNG
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);
    
// Init 16,384 RNG streams - each rank has an independent stream
    InitDefault();
    
// Note, use the mpi_myrank to select which RNG stream to use. 
    printf("Rank %d of %d has been started and a first Random Value of %lf\n", 
	   argc, mpi_commsize, GenVal(mpi_myrank));
    
    MPI_Barrier( MPI_COMM_WORLD );
// Bring over rest from your code
    g_mpi_myrank = mpi_myrank;

// DEAL WITH ARGS
    if( argc != 4)
    {
        printf("Incorrect number of args. You know what you did\n");
        return 1;
    }

    i = (unsigned)atoi(argv[1]);
    g_y_cell_size=i;
    g_x_cell_size = (i/mpi_commsize);
    if( g_y_cell_size < 3 )
    {
        printf("Size of map needs to be greater than 1\n");
        return 1;
    }

    if( (g_num_ticks = (unsigned)atoi(argv[2]) ) < 1 )
    {
        printf("At least 1 tick\n");
        return 1;
    }

    i = atoi(argv[3]);
    if( i < 0 )
    {
        printf("survival thresh hold must be at least 0%%\n");
        return 1;
    }

    g_thresh_hold = (double) (i/100.0);

//DONE DEALING WITH ARTGS
/*
    printf( "TOTAL SIZE: %u \n g_y_cell_size: %u\n g_x_cell_size %u\nGenVal(mpi_myrank):%lf\n",
        mpi_commsize, g_y_cell_size , g_x_cell_size , GenVal(mpi_myrank));
*/



// END -Perform a barrier and then leave MPI
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}

/***************************************************************************/
/* Other Functions - Bring over from your Assignment 1 *********************/
/***************************************************************************/

void allocate_and_init_cells()
{
  int i, j;
  // use "calloc" to allocate space for your cell matrix
  // use "drand48" to init the state of each grid cell once allocated.
  g_GOL_CELL = (unsigned char**)calloc(sizeof(unsigned char*) , g_x_cell_size+2);
  
  for( i = 1 ; i < g_x_cell_size ; i++)
  {
    g_GOL_CELL[i] = (unsigned char *)calloc(sizeof(unsigned char) , g_y_cell_size);
    for( j = 0; j < g_y_cell_size ; j++)
    {
      g_GOL_CELL[i][j] = RNG_check(INITAL_RNG_GAIN);
    }
  }
  return;   
}

void compute_one_tick()
{
  // iterate over X (outside loop) and Y (inside loop) dimensions of the g_GOL_CELL
  // Use drand48() for uniform distribution. It is already included in stdlib.h
  calculate_tick();
  //Insert Safty Error Check HERE
  execute_tick(); 

}

void random_setting()
{
  int i , j;
  for(i = 1; i < g_x_cell_size; i++)
  {
    for(j = 0; j < g_y_cell_size; j++)
    {
      g_GOL_CELL[i][j] = RNG_check(g_thresh_hold);
    }
  }
}

void calculate_tick()
{
  int i , j;
  for(i = 1; i < g_x_cell_size; i++)
  {
    for(j = 0; j < g_y_cell_size; j++)
    {
      g_GOL_CELL[i][j] = look_around(i,j);
    }
  }
}

unsigned char look_around(int x, int y)
{
  int count = 0; 
/*
  if (x > 0)
    if(g_GOL_CELL[x-1][y] > 1) count++;
  if (y > 0)
    if(g_GOL_CELL[x][y-1] > 1) count++;
  if(x < g_x_cell_size -2)
    if(g_GOL_CELL[x+1][y] > 1) count++;
  if(y < g_y_cell_size -2)
    if(g_GOL_CELL[x][y+1] > 1) count++;
*/

  int i , j; 
  //printf( " HERE AT [%d][%d] " , x, y);
  for(i = x-1; i<x+2 ; i++ )
  {
    //printf(" LOOKING AT [%d][%d] \n" , i , j);
    if( i <  0  ||  g_x_cell_size -1 <  i ) continue;
    for( j = y-1 ; j< y+2 ; j++)
    {
      //printf(" I'm IN [%d][%d] \n" , i , j);
      if( j < 0 || j > g_y_cell_size -1 ) continue;
      if( i == x && j == y ) continue;
      //printf( "CHECKING [%d][%d]  where i = %d and j = %d \n" , i , j , x , y);
      if( g_GOL_CELL[i][j] > 1 ) count++;
    }
  }
  //printf("\n");
  //exit(1);
  if( g_GOL_CELL[x][y] == ALIVE )
  {
    if ( count < 2 || count > 3) return TO_BE_DEAD;
    else return ALIVE;
  }
  else if( g_GOL_CELL[x][y] == DEAD)
  {
    if( count ==3 ) return TO_BE_ALIVE;
    else return DEAD;
  }
  else{
    fprintf( stderr , "ENCOUNTERED ERROR AT [%d][%d]==%d \n", x , y, g_GOL_CELL[x][y]);
    //print_cells(stderr);
    abort();
  }
  return -1;
}



void execute_tick()
{
  int i, j;
  for( i = 0 ; i < g_x_cell_size ; i++)
  {
    for( j = 0; j < g_y_cell_size ; j++)
    {
      if(g_GOL_CELL[i][j] == TO_BE_ALIVE)
        g_GOL_CELL[i][j] = ALIVE;
        
      else if(g_GOL_CELL[i][j] == TO_BE_DEAD)
          g_GOL_CELL[i][j] = DEAD; 
    }
  }
}

unsigned int RNG_check( double gain)
{
  return( GenVal(g_mpi_myrank) < gain )*ALIVE;
}