#Gestion des tâches standard périodiques
## Questions
* C'est le nombre de timers utilisés.
* Lorsqu'on n'a pas exécuter une tâche pendant 2 périodes.
* Pour "salut" en plus de "bonjour" il faut créer une variable de type Mess_st, l'initialiser avec un nouveau timer et le faire exécuter dans la fonction loop(). Il faut augmenter la macro MAX_WAIT_FOR_TIMER à 3.

#Utilisation de l'écran
## Questions
Ci-dessous notre code commenté : 

```c
int cpt = 0;
void setup() {
  //initialisation 
  Serial.begin(9600);
  //définir la police, couleur de l'écran
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32


}

void loop() {
  //nettoyer l'écran avant une nouvelle affichage 
  display.clearDisplay();
  //écrit sur l'écran
  display.println(cpt);
  //coordonnées sur l'écran 
  display.setCursor(0,0);
  //afficher
  display.display();
  delay(1000);
  //incrémente le compteur 
  cpt++;
```

# Communications inter-tâches
## Questions

Dans la fonction loop_T1: on appelle waitFor(0,500000), cela veut dire que la tâche associée à cette fonction s'executera toutes les demi secondes et le timer 0( vérifier qu'il n'est pas utilisé par une autre tâche), on lit la valeur de la résistance avec analoRead, si le mailbox est vide, alors on affecte à la variable val. 
Dans la fonction loop_Led, si le mailbox est vide, alors la fonction ne fait rien. Sinon, on change la période de la led en le multipliant par la valeur de la résistance et par 3000.
Il faut faire attention que la période et le val ont le même type.
Voici le code ci-dessous:

```c
enum state{EMPTY, FULL};
struct mailbox {
   enum state state;
   long val;
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



void loop_Led(struct Led_st * ctx, struct mailbox *mb) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  if (mb->state == EMPTY) return;
  ctx->period = mb->val*3000;
  Serial.println(ctx->period);
  digitalWrite(ctx->pin,ctx->etat);
  ctx->etat = 1-ctx->etat;
  mb->state = EMPTY;
}



//--------- Setup et Loop

void setup() {
   Serial.begin(9600);
}

void loop() {
  //Tâche pour lire la valeur de résistance. 
  loop_T1(&mb0);
  //Tâche pour faire clignoter la led.
  loop_Led(&Led1,&mb0);  
}
```


# Gestion des interruptions
## Questions
On utilise une variable globale pour indiquer que lorsqu'on appuie sur le caractère 's', elle devient 1.
```c
int stop = 0;
```


On ajoute cette ligne dans la fonction loop pour stopper le clignotement.
```c
void loop() {
	if(stop == 0){
		loop_Led(&Led1,&mb0);  
  		loop_T1(&mb0);
	}
}

```

L'arduino ne reconnait pas Serial.onReceive, on utilise alors une alternative: serialEvent. Cette fonction est appelée lorsqu'on envoie un caractère depuis le clavier.
On utilise Serial.read() pour lire un caractère tapé sur clavier et de le stocker dans la variable incomingByte, si elle est égale à 's', alors on change la valeur de stop.
```c
void serialEvent(){
  // Lire un caractère tapé sur clavier
  int incomingByte = Serial.read();
  if(incomingByte == 's'){
      stop = 1;
  }
}
```

