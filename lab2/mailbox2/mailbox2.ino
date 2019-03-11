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


//--------- Déclaration des tâches







unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta += 1 + (0xFFFFFFFF / period);    // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}
  
//--------- définition de la tache Led


enum state{EMPTY, FULL};
struct mailbox {
   enum state state;
   long val;
};


struct Led_st {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
}; 


struct Led_st Led1;
int stop = 0;
struct mailbox mb0 ={.state = EMPTY};





void setup_Led( struct Led_st * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void loop_Led(struct Led_st * ctx, struct mailbox *mb) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  if (mb->state == EMPTY) return;
  ctx->period = mb->val*3000;
  Serial.println(ctx->period);
  digitalWrite(ctx->pin,ctx->etat);
  ctx->etat = 1-ctx->etat;
  mb->state = EMPTY;
}



void loop_T1(struct mailbox *mb) {
  if (!waitFor(1,500000)) return;
  int v = analogRead(A1);
  if (mb->state != EMPTY) return; // attend que la mailbox soit vide
  mb->val = v;
  mb->state = FULL;
}





//--------- Setup et Loop

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  setup_Led(&Led1, 0,100000, 13);             
}


void serialEvent(){
  // Lire un caractère tapé sur clavier
  int incomingByte = Serial.read();
  if(incomingByte == 's'){
      stop = 1;
  }
}


void loop() {
	if(stop == 0){
		  loop_Led(&Led1,&mb0);  
  		loop_T1(&mb0);
	}
}
