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

#define INITAL_RNG_GAIN 0.5
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
int g_world_size;

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

// Bring over from your code
void allocate_and_init_cells();
void compute_one_tick();
void output_final_cell_state();
void print_cells(FILE* stream); //general thing for error seaching

void random_setting();
void transfer_ghosts();
void calculate_tick();
void execute_tick();
unsigned char look_around( int x , int y);

unsigned int RNG_check(double gain);
unsigned int RNG_n_check( int val , double gain);

/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
//    int i = 0;
    int mpi_myrank;
    int mpi_commsize;
    int i;
    double start_time =0, end_time;
// Example MPI startup and using CLCG4 RNG
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);
    
// Init 16,384 RNG streams - each rank has an independent stream
    InitDefault();
    
// Note, use the mpi_myrank to select which RNG stream to use. 
    printf("Rank %d of %d has been started and a first Random Value of %lf\n", 
	   mpi_myrank, mpi_commsize, GenVal(mpi_myrank));
// Bring over rest from your code
    g_mpi_myrank = mpi_myrank;
    g_world_size = mpi_commsize;

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
    
    MPI_Barrier( MPI_COMM_WORLD );
/* EXECUTION */
    if( MASTER == mpi_myrank ) start_time = MPI_Wtime();
    allocate_and_init_cells();

    for( i = 0 ; i< g_num_ticks ; i++)
    {
        compute_one_tick();
    }


    MPI_Barrier( MPI_COMM_WORLD );
    if( MASTER == mpi_myrank )
    {
        end_time = MPI_Wtime();
        printf( "TIME : %f sec\n" , end_time- start_time);
    }
/* PRINTING */
/*
// I'm doing it this way because I'm both lazy
//  and effective. Why should I bother to paste things together    
    FILE * of = NULL;

    if( g_mpi_myrank != MASTER)
    {
        MPI_Recv(&i , 1 , MPI_INT , mpi_myrank-1 , 0, 
          MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        
        of = fopen("OUT.log" , "a");
    }else{ of = fopen( "OUT.log" , "w"); }
     
    if( of == NULL )
    {
        printf( "Couldn't open file\n");
        MPI_Finalize();
        exit(1);
    }

    //fprintf(of," HELLO FROM %u\n" , g_mpi_myrank); 
    
    //printf("HELLO FROM %u\n", g_mpi_myrank);
    print_cells(of);
    fclose(of);
    
    if( g_mpi_myrank != mpi_commsize -1)
    {
        MPI_Send(&i , 1 , MPI_INT, mpi_myrank+1 , 0, MPI_COMM_WORLD);
    }
    
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
  
  for( i = 0 ; i < g_x_cell_size+2 ; i++)
  {
    g_GOL_CELL[i] = (unsigned char *)calloc(sizeof(unsigned char) , g_y_cell_size);
    for( j = 0; j < g_y_cell_size ; j++)
    {
      g_GOL_CELL[i][j] = RNG_n_check( (g_x_cell_size*g_mpi_myrank) + i ,  INITAL_RNG_GAIN);

    }
  }
  return;   
}

void compute_one_tick()
{
  // iterate over X (outside loop) and Y (inside loop) dimensions of the g_GOL_CELL
  // Use drand48() for uniform distribution. It is already included in stdlib.h
  int random;
  if( g_mpi_myrank == MASTER) random = RNG_check( g_thresh_hold);
  MPI_Bcast(&random , 1, MPI_INT , MASTER , MPI_COMM_WORLD);

  if( random )
  {
    random_setting();
    return;
  }
  transfer_ghosts();
  calculate_tick();
  //Insert Safty Error Check HERE
  execute_tick(); 

}

void random_setting()
{
  int i , j;
  for(i = 1; i < g_x_cell_size+1; i++)
  {
    for(j = 0; j < g_y_cell_size; j++)
    {
      g_GOL_CELL[i][j] = RNG_n_check( (g_x_cell_size*g_mpi_myrank) + i ,  .5);

    }
  }
}

void transfer_ghosts()
{
    int tag = g_mpi_myrank*2;
    MPI_Request R_Top_S, R_Top_R, R_Bot_S, R_Bot_R;
    MPI_Status  S_Top, S_Bot;
    //TOP SIDE
    if( g_mpi_myrank != MASTER )
    {
        MPI_Irecv( g_GOL_CELL[0], g_y_cell_size , MPI_UNSIGNED_CHAR ,
            g_mpi_myrank-1, tag-1, MPI_COMM_WORLD , &R_Top_R );

        MPI_Isend( g_GOL_CELL[1], g_y_cell_size , MPI_UNSIGNED_CHAR ,
            g_mpi_myrank-1, tag-1 , MPI_COMM_WORLD , &R_Top_S);
    }

    //BOTTOM SIDE
    if( g_mpi_myrank < g_world_size-1 )
    {
        MPI_Irecv( g_GOL_CELL[ g_x_cell_size +1], g_y_cell_size , MPI_UNSIGNED_CHAR ,
            g_mpi_myrank+1, tag+1, MPI_COMM_WORLD , &R_Bot_R );

        MPI_Isend( g_GOL_CELL[g_x_cell_size], g_y_cell_size , MPI_UNSIGNED_CHAR ,
            g_mpi_myrank+1, tag+1, MPI_COMM_WORLD , &R_Bot_S);
    }

    if(g_mpi_myrank != MASTER )
        MPI_Wait( &R_Top_R , &S_Top);
    if(g_mpi_myrank != g_world_size -1 )
        MPI_Wait( &R_Bot_R , &S_Bot);
}

void calculate_tick()
{
  int i , j;
  for(i = 1; i < g_x_cell_size+1; i++)
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
    if( i <  0  ||  g_x_cell_size -1 <  i ) continue; //Safty Measure. 
    for( j = y-1 ; j< y+2 ; j++)
    {
      //printf(" I'm IN [%d][%d] \n" , i , j);
      if( j < 0 || j > g_y_cell_size -1 ) continue; // Again, Safty Measure
      if( i == x && j == y ) continue;              // I am a defensive Coder
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



void execute_tick() // This exists from HMWK 1. The Homework specifies all changes occur
{                   // at the same time, this ensures that, by making the change in state
  int i, j;         // occur at a singular moment. 
  for( i = 0 ; i < g_x_cell_size+1 ; i++)
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

void print_cells(FILE *stream)
{
  int i,j, live_count=0;

if( LINED == 1)
{   
  fprintf(stream, "\n\n  |" );
  for(i = 0 ; i < g_x_cell_size+1 ; i++)
  { 
    fprintf(stream, " %d |", i);
  }
  fprintf(stream, "\n---");
 
  for( j = 0; j < g_x_cell_size+1 ; j++)
  {
    fprintf(stream,"----");
  }
  fprintf(stream,"\n"); 
 
   for( i = 0 ; i < g_y_cell_size ; i++)
  {
    fprintf(stream, "%d |", i);
    for( j = 0; j < g_x_cell_size+1 ; j++)
    {
      char printz = '1';
      if( g_GOL_CELL[i][j] == ALIVE ) printz = '0';
      fprintf(stream," %c |" , printz);
      live_count += g_GOL_CELL[j][i];
    }
    fprintf(stream,"\n---");
    for( j = 0; j < g_x_cell_size+1 ; j++)
    {
      fprintf(stream,"----");
    }
    fprintf(stream,"\n");
  }
  return;
}

  //fprintf(stream,"\n\n"); 
  //printf( "X: %u , Y: %u \n", g_x_cell_size , g_y_cell_size);
  for( i = 1 ; i < g_x_cell_size+1 ; i++)
  {
    for( j = 0; j < g_y_cell_size ; j++)
    {
      char printz = ' ';
      if( g_GOL_CELL[i][j] == ALIVE ) printz = 'X';
      fprintf(stream,"%c" , printz);
      live_count += g_GOL_CELL[j][i];
    }
    fprintf(stream,"\n");
  }
  return;
}

unsigned int RNG_n_check( int val , double gain)
{
  return( GenVal(val) < gain ) * ALIVE;
}


unsigned int RNG_check( double gain)
{
  return( GenVal(g_mpi_myrank) < gain )*ALIVE;
}
