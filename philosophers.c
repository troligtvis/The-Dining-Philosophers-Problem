/************************************************
 *    Program: philosophers.c
 *    Namn: Karl-Johan Drougge
 *    
 *    The Dining Philosophers Problem [Solved]
 *
 ************************************************/  

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char table[100] = "_"; // Beroende av varje tråd som skapas så får skrivs det ut "W_"
char *eat = "|E|";
char *think = "_T_";
char *wait = "W";
char *right_wait = "|W";
char *left_wait = "W|";

pthread_mutex_t kritisk_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t chopstick[10]=    //initierar mutexarna
{
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};

struct phil_parms
{
  int pos;
  int noPhils;
  int lock;
  int klar;
};

void grab_chopstick(int chop)
{
  pthread_mutex_lock(&chopstick[chop]);
}

void drop_chopstick(int chop)
{
  pthread_mutex_unlock(&chopstick[chop]);
}

void done_eating(int c1, int c2)
{
  pthread_mutex_unlock(&chopstick[c1]);
  pthread_mutex_unlock(&chopstick[c2]);
}

void just_thinking(int t_id, char chop, int antal)
{
  int i = 0, x = 0;
  if(chop == 'r') //kontrollera om de försöker tar den högra
  {  
    for(i = (2 * t_id); x < 2; i++)
    {
      table[i] = right_wait[x];
      x++;
    }
    
    if(t_id == 0)
    {
      table[antal*2] = '|';
    }
    return;
  }

  else
  {
    for(i = (2 * t_id); x < 2; i++)
    {
      table[i] = left_wait[x];
      x++;
    }

    if(t_id == antal-1)
    {
      table[0] = '|';
    }
    return;
  }
}

void ch_status(int t_id, int slump, char *status, int antal)
{
  int i, x = 0;

  //Ifall WAITING
  if(status == wait)
  {
    table[(2 * t_id) + 1] = 'W';
// _ W _ W _ W _ W _ W _
// 0 1 2 3 4 5 6 7 8 9 10

    if(t_id < antal-1)
    {
      table[t_id * 2] = '|';
    }
    else
    {
      table[0] = '|';
    }

    if(t_id == 0)
    {
      table[antal*2] = '|';
    }
    return;
  }

  //Ifall EATING
  else if(strcmp(status, eat) == 0)
  {
    for(i = (2 * t_id); x < 3; i++)
    {
      table[i] = status[x];
      x++;
    }

    if (t_id == antal-1) 
    {
      table[0] = '|';
    }
   return;
  }

  //Ifall THINKING
  else if(strcmp(status, think) == 0)
  {
    for(i = (2 * t_id); x < 3; i++)
    {
      table[i] = status[x];
      x++;
    }

    if (t_id == antal-1)
    {
      table[0] = '_';
    }
    return;
  }
}

void* philosophize (void* parameters)
{
  /*Put philosopher-code here*/
  struct phil_parms *p = (struct phil_parms*) parameters;
  int slump = (2 + rand() % 5);
  int t_id = p->pos;                      //tar positionen och sätter till t_id, tråd id.
  int right_chopstick = t_id;             //ger höger chopstick ett id
  int left_chopstick = t_id+1;            //ger vänster chopstick ett id

  if(left_chopstick == p->noPhils)        //om det är den sista filosofen så ska dens vänstra chopstick vara den förstas högra.
  {
    left_chopstick = 0;
  }

  // Ingen låsning och de turas om att äta.
  if(p->lock == 1)
  {
    while(p->klar)        //evig loop fram till att mainprogrammet kört igenom 48st rundor då ändras värdet på klar och den åker ur loopen.
    { 
      if(t_id == p->noPhils-1)
      {
        grab_chopstick(left_chopstick);
      }
      else
      {
        grab_chopstick(right_chopstick);
      }

      //VÄNTA~~~~~~~~~~~~~~~~~~~~~~~~~~VÄNTA
      pthread_mutex_lock(&kritisk_mutex);
      ch_status(t_id, slump, wait, p->noPhils);
      pthread_mutex_unlock(&kritisk_mutex);
      //~~~~~~~~~~~~KLAR~~~~~~~~~~~~~~~~~~~~

      if(t_id == p->noPhils-1)
      {
        grab_chopstick(right_chopstick);
      }
      else
      {
        grab_chopstick(left_chopstick);
      }

      //ÄTA~~~~~~~~~~~~~~~~~~~~~~~~~~ÄTA
      pthread_mutex_lock(&kritisk_mutex);
      ch_status(t_id, slump, eat, p->noPhils);
      pthread_mutex_unlock(&kritisk_mutex);
      //~~~~~~~~~~~~KLAR~~~~~~~~~~~~~~~~

      sleep(1 + rand() % 3);

      if(t_id == p->noPhils-1)
      {
        drop_chopstick(right_chopstick);
      }
      else
      {
        drop_chopstick(left_chopstick);
      }

      //TÄNKA~~~~~~~~~~~~~~~~~~~~~~~~~~TÄNKA
      pthread_mutex_lock(&kritisk_mutex);
      ch_status(t_id, slump, think, p->noPhils);
      pthread_mutex_unlock(&kritisk_mutex);
      //~~~~~~~~~~~~~~~KLAR~~~~~~~~~~~~~~~~~

      if(t_id == p->noPhils-1)
      {
        drop_chopstick(left_chopstick);
      }
      else
      {
        drop_chopstick(right_chopstick);
      }

      pthread_mutex_lock( &kritisk_mutex );
      if (t_id <  p->noPhils-1)
      {
        table[t_id*2] = '_';
      }
      else
      {
        table[0] = '_';
      }
        
      if (t_id == 0)
      {
        table[p->noPhils*2] = '_';
      }
            
      if (t_id == p->noPhils-1)
      {
        table[p->noPhils*2] = '_';
      }       
      pthread_mutex_unlock( &kritisk_mutex );

      sleep(slump);

    }//while-loopen med done!
  }

  //Deadlock med hungriga filosofer
  else
  {
    while(p->klar)
    {
      char right = 'r';
      grab_chopstick(right_chopstick);

      //VÄNTA~~~~~~~~~~~~~~~~~~~~~~~~~~VÄNTA
      pthread_mutex_lock(&kritisk_mutex);
      just_thinking(t_id, right, p->noPhils);
      pthread_mutex_unlock(&kritisk_mutex);
      //~~~~~~~~~~~~KLAR~~~~~~~~~~~~~~~~~~~~
      
      sleep(1 + rand() % 3);

      grab_chopstick(left_chopstick);

      //ÄTA~~~~~~~~~~~~~~~~~~~~~~~~~~ÄTA
      pthread_mutex_lock(&kritisk_mutex);
      ch_status(t_id, slump, eat, p->noPhils);
      pthread_mutex_unlock(&kritisk_mutex);
      //~~~~~~~~~~~~KLAR~~~~~~~~~~~~~~~~

    }
  }

  return NULL;
}


int main (int argc, char* argv[])
{ 
  int noPhils, lock;
  noPhils = atoi(argv[1]);  // första argumentet ska vara hur många filosofer.
  lock = atoi(argv[2]);     // om man ska få en låsning eller inte
  pthread_t phils[noPhils]; // antalet philosofer
  struct phil_parms control_phil[noPhils];
  int _round = 0;
  int i = 0;
 
  srand(time(NULL));

  for (i = 0; i < noPhils; i++)
  {
    //initerar mutex för alla chopsticks.
    pthread_mutex_init(&chopstick[i], NULL);

  }

  for(i = 0; i < noPhils; i++)
  {
    strcat(table, "T_");        //appendar på "T_" för varje filosof.        
    control_phil[i].pos = i;   //sätter in vilken position varje trådfilosof har
    control_phil[i].noPhils = noPhils; //antalet filosofer
    control_phil[i].lock = lock;      // 0=lås, 1=ej lås
    control_phil[i].klar = 1;        //fastna i en loop tills det gått 48 antal rundor
  }

  for(i = 0; i < noPhils; i++)
  {
    pthread_create(&phils[i], NULL, philosophize, &control_phil[i]);
  }

  while(_round<48)
  {
    printf("Round %2d: %s\n", _round+1, table);
    sleep(1);
    _round++;
  } 

  printf("ALLA NÖJDA GLADA!\n");

  //samlar in alla trådar och avslutar dom.
  for(i = 0; i < noPhils; i++)
  {
    control_phil[i].klar = 0;
    pthread_join(phils[i], NULL);
  }

  return 0;
}
