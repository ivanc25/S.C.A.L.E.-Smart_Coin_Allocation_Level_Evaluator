# 🐖 S.C.A.L.E. PiggyBank
**S**mart **C**oin **A**llocation & **L**evel **E**valuator

![STM32](https://img.shields.io/badge/STM32-WB09-blue) ![BLE](https://img.shields.io/badge/Bluetooth-5.4-blue) ![C](https://img.shields.io/badge/Language-C-orange)

## 📖 Descrizione del Progetto
**S.C.A.L.E. PiggyBank** è un salvadanaio IoT intelligente basato su architettura **Edge-Computing**. Sviluppato per un progetto accademico presso il Politecnico di Bari, il sistema trasforma un oggetto passivo in un nodo interconnesso, capace di smistare meccanicamente le monete, calcolarne il valore tramite acquisizione gravimetrica ad alta precisione e comunicare il saldo in tempo reale via Bluetooth Low Energy (BLE) a una Web App dedicata.

## ✨ Funzionalità Principali
* **Smistamento Meccanico Passivo:** Scivolo calibrato in PVC con 5 feritoie specifiche per separare le monete in base al diametro, eliminando la necessità di sensori ottici complessi.
* **Digitalizzazione Gravimetrica:** Misurazione continua del peso tramite 5 celle di carico (1kg) interfacciate con ADC HX711 a 24-bit per una precisione milligrammatica.
* **Sistema Anti-Inceppamento (Jamming):** Motore a vibrazione ERM (Eccentric Rotating Mass) e doppio stadio di baffle per singolarizzare le monete in ingresso nella tramoggia.
* **Edge Computing:** Elaborazione del segnale grezzo, filtraggio del rumore e calcolo del risparmio totale gestiti direttamente a bordo (on-edge) dal microcontrollore.
* **Connettività BLE 5.4:** Trasmissione a bassa latenza tramite protocollo GATT verso una Web App cross-platform (zero-installation).
* **Controllo Accesso Attivo:** Chiusura di sicurezza gestita da un servomotore PWM (TG9e) che si sblocca solo al raggiungimento della soglia di risparmio impostata dall'utente.

## 🛠️ Architettura Hardware
Il "cervello" del sistema è l'**STM32WB09** (Arm Cortex-M0+ @ 64 MHz), scelto per la sua capacità di gestire parallelamente l'acquisizione sensoriale e lo stack di comunicazione radio senza interruzioni.

**Panoramica Componenti:**
- **MCU:** STMicroelectronics STM32WB09 (con coprocessore radio DMA-based)
- **Sensori:** 5x Celle di Carico Cantilever + 5x ADC HX711
- **Attuatori:** Servomotore TG9e, Motore DC ERM
- **Power Management:** Architettura a *Dual Power Domain* con separazione galvanica (9V per Logica/Celle, 6V dedicati all'isolamento dei motori per sopprimere i disturbi di commutazione).

## 💻 Logica del Firmware
Il firmware in C nativo è basato su un **paradigma di esecuzione cooperativa**, implementando una macchina a stati per i seguenti task principali:
1.  **Tara Task:** Media di 20 campioni eseguiti all'avvio per azzerare l'offset di peso dei singoli scomparti meccanici.
2.  **Read Task:** Acquisizione in bit-banging fortemente sincronizzata su 5 canali DOUT (con clock unificato).
3.  **Logic Task:** Smoothing dei dati tramite filtro a media mobile (5 sample) e quantizzazione (arrotondamento) sui valori standard delle monete metalliche.
4.  **BLE & PWM Task:** Aggiornamento asincrono delle caratteristiche GATT per la notifica alla Web App e regolazione del timer hardware per il segnale PWM del servo (periodo 20ms).

## 📂 Struttura della Repository
```text
├── 📁 File_Mx_IDE/           # Codice sorgente C per MCU STM32WB09 (Progetto STM32CubeIDE)
├── 📁 Docs/                  # Relazione di progetto e slide della presentazione
├── 📄 index.html             # Frontend HTML/JS interfacciato tramite Web Bluetooth API
└── 📄 README.md
