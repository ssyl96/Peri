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
  
//--------- définition de la tache Led

struct Led_st {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
}; 

void setup_Led( struct Led_st * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void loop_Led(struct Led_st * ctx) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;                              // changement d'état
}

//--------- definition de la tache Mess

struct Mess_st {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                             // periode d'affichage
  char mess[20];
} Mess_t ; 

void setup_Mess(struct Mess_st * ctx, int timer, unsigned long period, const char * mess) {
  ctx->timer = timer;
  ctx->period = period;
  strcpy(ctx->mess, mess);
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
}

void loop_Mess(struct Mess_st *ctx) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  Serial.println(ctx->mess);                              // affichage du message
}

//--------- Déclaration des tâches

struct Led_st Led1;
struct Mess_st Mess1;
struct Mess_st Mess2;

//--------- Setup et Loop

void setup() {
  setup_Led(&Led1, 0, 100000, 13);                        // Led est exécutée toutes les 100ms 
  setup_Mess(&Mess1, 1, 1000000, "bonjour");              // Mess est exécutée toutes les secondes 
  setup_Mess(&Mess2, 2, 1500000,"salut");
}

void loop() {
  loop_Led(&Led1);                                        
  loop_Mess(&Mess1); 
  loop_Mess(&Mess2);
}
