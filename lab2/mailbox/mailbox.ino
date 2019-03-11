// --------------------------------------------------------------------------------------------------------------------
// Multi-tâches cooperatives : solution basique
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// unsigned int waitFor(timer, period) 
// Timer pour taches périodiques 
// configuration :
//  - MAX_WAIT_FOR_TIMER : nombre maximum de timers utilisés
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de période écoulée depuis le dernier appel
// --------------------------------------------------------------------------------------------------------------------
#define MAX_WAIT_FOR_TIMER 3
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta += 1 + (0xFFFFFFFF / period);    // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}

enum state{EMPTY, FULL};
struct mailbox {
   enum state state;
   int val;
};

struct mailbox mb0 ={.state = EMPTY};


void loop_T1(struct mailbox *mb) {
  //lire toutes les demi-secondes 
  if (!waitFor(0,500000)) return;
  int v = analogRead(A1);
  if (mb->state != EMPTY) return; // attend que la mailbox soit vide
  mb->val = v;
  mb->state = FULL;
}


void loop_T2(struct mailbox *mb) {
  //récupérer la valeur toutes les demi-secondes
  if (!waitFor(1,500000)) return;
  int v =0;
  if (mb->state != FULL) return; // attend que la mailbox soit pleine
  v = mb->val; 
  mb->state = EMPTY;
}



//--------- Setup et Loop

void setup() {
   Serial.begin(9600);
}

void loop() {
  loop_T1(&mb0);
  loop_T2(&mb0);
}
