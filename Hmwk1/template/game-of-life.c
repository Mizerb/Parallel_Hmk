/***************************************************************************/
/* Template for Asssignment 1 **********************************************/
/* Benjamin Andrew Mizera     **********************************************/
/***************************************************************************/

/***************************************************************************/
/* Includes ****************************************************************/
/***************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

/***************************************************************************/
/* Defines *****************************************************************/
/***************************************************************************/

#define ALIVE 3
#define TO_BE_ALIVE 1 
#define DEAD  0
#define TO_BE_DEAD 2

#define INITAL_RNG_GAIN 0.2
/***************************************************************************/
/* Global Vars *************************************************************/
/***************************************************************************/

unsigned int **g_GOL_CELL=NULL;

unsigned int g_x_cell_size=0;
unsigned int g_y_cell_size=0;

unsigned int g_num_ticks=0;

double g_thresh_hold=0.0;

/***************************************************************************/
/* Function Decs ***********************************************************/
/***************************************************************************/

void allocate_and_init_cells();
void compute_one_tick();
void output_final_cell_state();
void print_cells(FILE* stream); //general thing for error seaching
unsigned int RNG_check(double gain);
/***************************************************************************/
/* Function: Main **********************************************************/
/***************************************************************************/

int main(int argc, char *argv[])
{
  int i = 0;

  // insert your code to capture inputs from argc and argv
  // Argument 1 is size of X and Y cell dimensions
  // Argument 2 is the number of ticks
  // Argument 3 is the thresh hold percent 0%, 25%, 50%, 75% and 90%.
  // Convert strings to integer values using "atoi()"
  if( argc != 4)
  {
    printf("Incorrect number of args. You know what you did\n");
    return 1;
  }
  
  g_x_cell_size= (unsigned)atoi(argv[1]);
  g_y_cell_size=g_x_cell_size;
  if( g_x_cell_size < 1 )
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

// END OF DEALING WITH ARGS
//=========================================================================  

  allocate_and_init_cells();

  print_cells(stderr);

  for(i = 0; i < g_num_ticks; i++)
    {
      compute_one_tick();
    }

  output_final_cell_state();
  print_cells(stdout);
  return 0;
}

/***************************************************************************/
/* Function: allocate_and_init_cells ***************************************/
/***************************************************************************/

void allocate_and_init_cells()
{
  int i, j;
  // use "calloc" to allocate space for your cell matrix
  // use "drand48" to init the state of each grid cell once allocated.
   g_GOL_CELL = (unsigned int**)calloc(sizeof(unsigned int*) , g_x_cell_size);
  
  for( i = 0 ; i < g_x_cell_size ; i++)
  {
    g_GOL_CELL[i] = (unsigned int *)calloc(sizeof(unsigned int) , g_y_cell_size);
    for( j = 0; j < g_y_cell_size ; j++)
    {
      g_GOL_CELL[i][j] = RNG_check(INITAL_RNG_GAIN);
    }
  }
  return;   
}

/***************************************************************************/
/* Function: compute_one_tick **********************************************/
/***************************************************************************/

void calculate_tick();
void execute_tick();
int look_around( int x , int y);

void compute_one_tick()
{
  // iterate over X (outside loop) and Y (inside loop) dimensions of the g_GOL_CELL
  // Use drand48() for uniform distribution. It is already included in stdlib.h
  calculate_tick();
  //Insert Safty Error Check HERE
  execute_tick(); 

}

void calculate_tick()
{
  int i , j;
  for(i = 0; i < g_x_cell_size; i++)
  {
    for(j = 0; j < g_y_cell_size; j++)
    {
      g_GOL_CELL[i][j] = look_around(i,j);
    }
  }
}

int look_around(int x, int y)
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
    print_cells(stderr);
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

/***************************************************************************/
/* Function: output_final_cell_state ***************************************/
/***************************************************************************/

void output_final_cell_state()
{
  // print out in grid form 16 value per row of the g_GOL_CELL
  // This data will be used to create your graphs
}


/***************************************************************************/
/* Function: print_cells ***************************************************/
/***************************************************************************/

void print_cells(FILE *stream)
{
  int i,j, live_count=0;
  
  fprintf(stream, "  |" );
  for(i = 0 ; i < g_x_cell_size ; i++)
  { 
    fprintf(stream, " %d |", i);
  }
  fprintf(stream, "\n---");
 
  for( j = 0; j < g_x_cell_size ; j++)
  {
    fprintf(stream,"----");
  }
  fprintf(stream,"\n"); 
 
   for( i = 0 ; i < g_y_cell_size ; i++)
  {
    fprintf(stream, "%d |", i);
    for( j = 0; j < g_x_cell_size ; j++)
    {
      fprintf(stream," %u |" , g_GOL_CELL[j][i]);
      live_count += g_GOL_CELL[j][i];
    }
    fprintf(stream,"\n---");
    for( j = 0; j < g_x_cell_size ; j++)
    {
      fprintf(stream,"----");
    }
    fprintf(stream,"\n");
  }

}


/***************************************************************************/
/* Function: RNG_check ****************************************************/
/***************************************************************************/

unsigned int RNG_check( double gain)
{
  return( drand48() < gain )*ALIVE;
}

