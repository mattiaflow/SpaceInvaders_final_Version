# Space Invaders per Arduino

Un piccolo gioco arcade ispirato a **Space Invaders**, realizzato per Arduino e visualizzato su un display OLED SH1106 da 1,3 pollici (128×64 pixel).

Il giocatore muove la navicella con un potenziometro e spara con un pulsante. I nemici diventano progressivamente più veloci e frequenti; un secondo tipo di alieno compare dopo 20 eliminazioni.

## Funzionalità

- movimento analogico della navicella;
- pulsante di fuoco con gestione della pressione;
- difficoltà progressiva;
- due tipi di nemici;
- punteggio e sistema di vite;
- una vita bonus ogni 30 punti, fino a un massimo di 5;
- vittoria al raggiungimento di 80 eliminazioni;
- schermate di avvio e fine partita.

## Componenti

- una scheda Arduino compatibile;
- un display OLED SH1106 I²C, 128×64 pixel;
- un potenziometro;
- un pulsante;
- cavetti e breadboard.

## Collegamenti

| Componente | Collegamento Arduino |
| --- | --- |
| Cursore centrale del potenziometro | A1 |
| Estremi del potenziometro | 5V e GND |
| Pulsante | D7 e GND |
| OLED SDA | pin SDA della scheda |
| OLED SCL | pin SCL della scheda |
| OLED VCC | alimentazione compatibile con il modulo |
| OLED GND | GND |

Il pulsante usa la resistenza interna `INPUT_PULLUP`, quindi non serve una resistenza di pull-up esterna. Verifica sempre la tensione supportata dal tuo modulo OLED prima di alimentarlo.

## Dipendenze

- [U8g2](https://github.com/olikraus/u8g2), installata automaticamente da PlatformIO;
- `Wire`, inclusa nel core Arduino.

## Installazione e caricamento con PlatformIO

1. Installa l'estensione **PlatformIO IDE** in Visual Studio Code.
2. Scarica o clona questo repository e apri la sua cartella in VS Code.
3. Attendi che PlatformIO installi automaticamente la piattaforma AVR e la libreria U8g2.
4. Collega Arduino Nano e usa **PlatformIO: Upload**.
5. Per leggere i messaggi seriali usa **PlatformIO: Serial Monitor**; la velocità è già impostata a 9600 baud.

La configurazione predefinita usa `nanoatmega328`, adatta ai Nano ATmega328P con bootloader classico. Se il caricamento non riesce e la tua scheda usa il nuovo bootloader, seleziona l'ambiente `nano_new` dalla barra di PlatformIO oppure esegui:

```bash
pio run -e nano_new -t upload
```

Per compilare senza caricare sulla scheda:

```bash
pio run
```

## Come si gioca

- Ruota il potenziometro per muovere la navicella a sinistra e a destra.
- Premi il pulsante per iniziare e per sparare.
- Ogni alieno eliminato vale un punto.
- Un alieno che supera il bordo destro fa perdere una vita.
- La partita termina quando le vite arrivano a zero oppure quando raggiungi 80 punti.

## Configurazione

I principali parametri di gioco si trovano all'inizio dello sketch:

```cpp
const byte buttonFire = 7;
const int asseX = A1;
const byte MAX_POINTS = 80;
const byte INITIAL_LIVES = 5;
```

Puoi modificarli per cambiare i pin, la durata della partita o il numero iniziale di vite.

## Struttura del progetto

```text
SpaceInvaders_final_Version/
├── src/
│   └── SpaceInvaders_final_Version.ino
├── platformio.ini
├── README.md
└── .gitignore
```

## Autore

Mattia Floriani — versione 1.7, gennaio 2026.

## Licenza

