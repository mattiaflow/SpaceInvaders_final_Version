/**
 * SPACE INVADERS - Arduino Nano OLED Game
 * @author MATTIA FLORIANI
 * @version 1.7 - 23/01/2026
 * Schermo: 1.3" OLED SH1106 (128x64px)
 * 
 * DESCRIZIONE DEL GIOCO:
 * Gioco sparatutto retrò ispirato a Space Invaders. Il giocatore controlla una navicella
 * triangolare posizionata nella parte bassa dello schermo e deve abbattere i nemici che
 * scendono dall'alto. I controlli sono:
 * - Potenziometro analogico (A1): muove la nave sinistra/destra
 * - Pulsante (PIN 7): spara missili verticali verso i nemici
 * 
 * MECCANICHE DI GIOCO:
 * • Primo alieno (👤): appare continuamente, velocità aumenta ogni 10 punti (max 6px/frame)
 * • Secondo alieno (👥): sblocca a 20 kill, velocità minore (max 4px/frame)
 * • Ogni nemico eliminato = +1 punto
 * • Nemici non abbattuti = -1 vita
 * • Bonus vita: +1 ogni 30 punti (massimo 5 vite totali)
 * • Vittoria: raggiungi 80 kill
 * • Sconfitta: perdi tutte le 5 vite
 * 
 * DIFFICOLTÀ PROGRESSIVA:
 * • Velocità nemici: aumenta ogni 10 punti
 * • Frequenza spawn: nemici più frequenti man mano che aumenta il punteggio
 * • Velocità proiettili: aumenta ogni 15 punti (max 8px/frame)
 * • Il gioco raggiunge il massimo carico hardware intorno ai 40-45 kill
 * 
 * STATI DEL GIOCO:
 * - IDLE: schermata di presentazione
 * - RUN: gameplay attivo
 * - GAMEOVER: risultati finali e attesa (5 secondi) prima di tornare a IDLE
 * 
 * HARDWARE:
 * • Display: U8G2 (I2C SH1106 128x64)
 * • Joystick: Potenziometro analogico su A1 + Pulsante su PIN 7
 * • Risoluzione: 128x64 pixel
 */
#include <U8g2lib.h>
#include <Wire.h>
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

enum state { idle,
             run,
             gameover };
state idState = idle;


//DICHIARAZIONE SERVIZZI FISICI
const byte buttonFire = 7;
const int asseX = A1;


//COSTANTI
const byte SCREEN_WIDTH = 128;
const byte SCREEN_HEIGHT = 64;
const byte BORDER_Y = 11;
const byte SHIP_Y = 62;
const byte SHIP_SIZE = 5;
const byte ALIEN_SIZE = 8;
const byte ALIEN_SPAWN_MIN_Y = 18;  //pavimento
const byte ALIEN_SPAWN_MAX_Y = 45;  //soffito
const byte INITIAL_ALIEN_SPEED = 1;
const byte INITIAL_MISSILE_SPEED = 3;
const unsigned int EXPLOSION_DURATION = 70;
const byte SPAWN_CHANCE = 40;  // Ridotto per difficoltà iniziale più facile
const byte SPAWN_CHANCE_2 = 30;  //possibilità
const byte MAX_POINTS = 80;
const byte INITIAL_LIVES = 5;  // Aumentato da 4 a 5

//DICHIARAZIONE SERVIZZI LOGICI
byte xship = SCREEN_WIDTH / 2;
byte yship = SHIP_Y;

byte fire;
byte xshoot;
byte yshoot;
bool FIRING = false;

byte xalien;
byte yalien;
bool ALIEN = false;

byte xalien2;
byte yalien2;
bool ALIEN2 = false;

byte alienSpeed = INITIAL_ALIEN_SPEED;
byte alienSpeed_2 = 2;
byte missileSpeed = INITIAL_MISSILE_SPEED;
bool BOOM = false;
byte boomAlien = 0;  // 1 = primo alieno, 2 = secondo alieno

unsigned long t1, dt;

byte points = 0;
byte UP = INITIAL_LIVES;
byte lastLevelReached = 0;

//gestione bottone
const unsigned long attesaPressione = 300;
const unsigned long ritardoPressione = 20;

//melodie


//----------SETUP----------
void setup() {
  Serial.begin(9600);
  oled.begin();
  pinMode(asseX, INPUT);
  pinMode(buttonFire, INPUT_PULLUP);
  randomSeed(analogRead(A3));

  oled.setFont(u8g2_font_likeminecraft_te);
  //https://github.com/olikraus/u8g2/wiki/fntpic/u8g2_font_likeminecraft_te.png
  //https://github.com/olikraus/u8g2/wiki/u8g2reference#drawtriangle
  oled.clearBuffer();
}

//----------LOOP----------
void loop() {
  oled.clearBuffer();
  //gestione stati
  switch (idState) {
    case idle:
      if (gestioneBottone(buttonFire)) idState = run;
      break;
    case run:
      if (points == MAX_POINTS || UP == 0) idState = gameover;
      break;
    case gameover:
      delay(5000); ///5 secondi di attesa
      idState = idle;      //dopo 5 secondi passaggio a idle
      UP = INITIAL_LIVES;  //ricarico le vite
      missileSpeed = INITIAL_MISSILE_SPEED;;
      points = 0;
      break;
  }

  //gestione funzioni
  switch (idState) {
    case idle:  // in fase di presentazione attesa
      Presentazione();
      break;

    case run:  //in fase di gioco
      Potenziomentro();
      Cornice();
      Sparo();
      Astronave();
      Missile();
      Alieno();
      Collisione();
      Esplosione();
      Punteggio();
      // Reset secondo alieno se scende sotto 20 punti
      if (points < 20) {
        ALIEN2 = false;
      }
      break;

    case gameover:  //fase di fine gioco
      GameOver();
      break;
  }
  oled.sendBuffer();

  //print Seriale
  Serial.print("Score ");
  Serial.print(points);
  Serial.print(", UP ");
  Serial.println(UP);
  Serial.print("faseId: ");
  Serial.println(idState + 1);
  
  delay(50);
}
//--------Funzioni---------

void Presentazione() {

  // Disegna presentazione
  //oled.drawFrame(0, 0, 128, 64);
  oled.drawStr(25, 12, "SPACE INVADERS");
  oled.drawTriangle(64 - SHIP_SIZE, yship, 64 + SHIP_SIZE, yship, 64, yship - SHIP_SIZE);
  oled.drawGlyph(29, 44, 0x00a4);
  oled.drawStr(34, 25, "<<press fire>>");
  oled.drawGlyph(82, 52, 0x00a5);
  oled.drawGlyph(63, 46, 0x00a7);
  oled.drawLine(0, 63, SCREEN_WIDTH, 63);
}

void GameOver() {

  //oled.drawFrame(10, 5, 118, 59);
  oled.setCursor(15, 20);
  oled.println("GAME OVER");
  oled.setCursor(23, 40);
  oled.print("kill ");
  oled.println(points);
  oled.drawTriangle(90,20,120,55,60,55);
  oled.drawBox(90, 3, 1, 12);
  
}


void Potenziomentro() {
  int v = analogRead(asseX);
  xship = map(v, 0, 1024, SCREEN_WIDTH - 5, 5);
}

void Cornice() {
  //oled.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  //oled.drawLine(0, BORDER_Y, SCREEN_WIDTH, BORDER_Y);
  oled.drawLine(0, 63, SCREEN_WIDTH, 63);
}

void Sparo() {
  //sparo
  if (gestioneBottone(buttonFire) && !FIRING) {
    FIRING = true;
    xshoot = xship;
    yshoot = yship - SHIP_SIZE;
  }
}

void Astronave() {
  //astronave
  oled.drawTriangle(xship - SHIP_SIZE, yship, xship + SHIP_SIZE, yship, xship, yship - SHIP_SIZE);
}

void Missile() {
  //missile
  missileSpeed = min(INITIAL_MISSILE_SPEED + (points / 15), 8); // Limita velocità massima

  if (FIRING) {
    oled.drawLine(xshoot, yshoot, xshoot, yshoot - SHIP_SIZE);
    yshoot = yshoot - missileSpeed;
    if (yshoot <= BORDER_Y + 8) {  //limite con il soffitto
      FIRING = false;
    }
  }
}

void Alieno() {
  // Aggiorna velocità ogni 10 punti
  alienSpeed = min((byte)(INITIAL_ALIEN_SPEED + (points / 10)), 6); // Limita velocità massima
  byte spawnChance = max((byte)(SPAWN_CHANCE - (points / 5)), 20); // Impedisce spawn chance troppo alto

  alienSpeed_2 = min((byte)(INITIAL_ALIEN_SPEED + (points / 25)), 4); // Limita velocità massima
  byte spawn_chance_2 = max((byte)(SPAWN_CHANCE_2 - (points / 20)), 20); // Usa nome diverso da costante

  //Primo Alieno
  if ((random(100) > spawnChance) && !ALIEN) {  //probabilità di comparsa
    ALIEN = true;
    xalien = 0;
    yalien = random(ALIEN_SPAWN_MIN_Y, ALIEN_SPAWN_MAX_Y);
  }

  if (ALIEN) {
    oled.drawGlyph(xalien, yalien, 0x00a4);
    if (!BOOM) {
      xalien = xalien + alienSpeed;
    }
    if (xalien >= SCREEN_WIDTH - 1) {
      ALIEN = false;
      UP--;
    }
  }

  //Secondo Alieno (appare a 20 punti)
  if (points >= 20) {
    if ((random(100) > spawn_chance_2) && !ALIEN2) {  //probabilità di comparsa
      ALIEN2 = true;
      xalien2 = 0;
      yalien2 = random(ALIEN_SPAWN_MIN_Y, ALIEN_SPAWN_MAX_Y);
    }

    if (ALIEN2) {
      oled.drawGlyph(xalien2, yalien2, 0x00a5);
      if (!BOOM) {
        xalien2 = xalien2 + alienSpeed_2;
      }
      if (xalien2 >= SCREEN_WIDTH - 1) {
        ALIEN2 = false;
        UP--;
      }
    }
  }
}
void Collisione() {
  //collisione - controlla se il missile colpisce l'alieno
  if (FIRING && ALIEN) {
    bool hitX = (xshoot >= xalien) && (xshoot <= xalien + ALIEN_SIZE);
    bool hitY = (yshoot >= yalien) && (yshoot <= yalien + ALIEN_SIZE);
    if (hitX && hitY) {
      BOOM = true;
      boomAlien = 1;
      FIRING = false;
      t1 = millis();
    }
  }

  //collisione - controlla se il missile colpisce il secondo alieno
  if (FIRING && ALIEN2) {
    bool hitX = (xshoot >= xalien2) && (xshoot <= xalien2 + ALIEN_SIZE);
    bool hitY = (yshoot >= yalien2) && (yshoot <= yalien2 + ALIEN_SIZE);
    if (hitX && hitY) {
      BOOM = true;
      boomAlien = 2;
      FIRING = false;
      t1 = millis();
    }
  }
}
void Esplosione() {
  //esplosione primo alieno
  if (BOOM && boomAlien == 1) {
    oled.drawCircle(xalien + 4, yalien - 4, 5);
    oled.drawCircle(xalien + 4, yalien - 4, 9);

    dt = millis() - t1;
    if (dt >= EXPLOSION_DURATION) {
      BOOM = false;
      boomAlien = 0;
      ALIEN = false;
      points++;
    }
  }
  //esplosione secondo alieno
  if (BOOM && boomAlien == 2) {
    oled.drawCircle(xalien2 + 4, yalien2 - 4, 5);
    oled.drawCircle(xalien2 + 4, yalien2 - 4, 9);

    dt = millis() - t1;
    if (dt >= EXPLOSION_DURATION) {
      BOOM = false;
      boomAlien = 0;
      ALIEN2 = false;
      points++;
    }
  }
}

void Punteggio() {
  //punteggio
  oled.setCursor(4, 9);  //X,Y
  oled.print("Kill ");
  oled.println(points);
  //vite e livelli
  oled.setCursor(70, 9);  //X,Y
  oled.print("up ");
  oled.setCursor(90, 11);
  switch (UP) {
    case 5: oled.print("*****"); break;
    case 4: oled.print("****"); break;
    case 3: oled.print("***"); break;
    case 2: oled.print("**"); break;
    case 1: oled.print("*"); break;
    case 0: break;
  }
  byte currentLevel = points / 30;
if (currentLevel > lastLevelReached && UP < INITIAL_LIVES) {
  UP++;
  lastLevelReached = currentLevel;
}
}


//gestione bottone
bool gestioneBottone(int pulsante) {
  unsigned long inizioPressione = millis();
  // Se il pulsante non è premuto, esci subito
  if (digitalRead(pulsante) != LOW) {
    return false;
  }
  // Se è premuto, controlla la durata
  while (digitalRead(pulsante) == LOW) {
    unsigned long durata = millis() - inizioPressione;
    if (durata > attesaPressione) {
      // Timeout: reset e uscita
      inizioPressione = millis();
      break;
    }
    if (durata > ritardoPressione) {
      // Pressione valida
      return true;
    }
  }
  // Se siamo usciti dal ciclo ma il pulsante era premuto
  return true;
}
