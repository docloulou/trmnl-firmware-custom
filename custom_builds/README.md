# Firmwares TRMNL pour XIAO ePaper Display Board EE04

Firmware TRMNL 1.8.16 compilé pour la carte **XIAO ePaper Display Board (ESP32-S3) – EE04**
(XIAO ESP32-S3 Plus), une version par écran :

| Dossier | Écran | Environnement PlatformIO | Modèle envoyé au serveur |
|---|---|---|---|
| `EE04_7inch5_BW/` | Waveshare 7.5" V2, 800x480, noir et blanc | `EE04_7inch5_BW` | `xiao_epaper_display` |
| `EE04_7inch3_Spectra6/` | Waveshare 7.3" e-Paper (E), 800x480, Spectra 6 | `EE04_7inch3_Spectra6` | `xiao_epaper_6clr` |

Chaque dossier contient :

- `merged_firmware.bin` : image complète (bootloader, partitions et application), à flasher à l'adresse **0x0**.
  C'est celle à utiliser pour le premier flash.
- `firmware.bin` : l'application seule, à flasher à l'adresse **0x10000** (mise à jour d'une carte déjà flashée).

## Brochage utilisé (EE04)

| Fonction | GPIO |
|---|---|
| SPI SCK / MOSI | 7 / 9 |
| CS / DC / RST / BUSY | 44 / 10 / 38 / 4 |
| Alimentation écran (mise à l'état haut au démarrage) | 43 |
| Bouton TRMNL (KEY2) | 5 |
| Batterie : ADC / activation de la mesure | 1 / 6 |

## Branchement de l'écran

- Branche la nappe de l'écran directement sur le connecteur FPC **24 broches** de l'EE04.
  Le HAT Waveshare (e-Paper Driver HAT) ne sert pas.
- Pour le 7.5", il faut la version **V2** (800x480, inscrite au dos de l'écran). La V1 (640x384) et
  la version (B) tricolore ne sont pas compatibles avec ce firmware.

## Flasher

1. Mets l'ESP32-S3 en mode bootloader : maintiens le bouton **BOOT** du module XIAO, branche l'USB-C
   (ou appuie sur RESET), puis relâche BOOT.
2. Flashe l'image complète, par exemple avec esptool :

   ```sh
   esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 460800 erase_flash
   esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 460800 write_flash 0x0 merged_firmware.bin
   ```

   Tu peux aussi passer par le flasheur web https://espressif.github.io/esptool-js/ (Chrome ou Edge) :
   `merged_firmware.bin` à l'adresse `0x0`.
3. Appuie sur RESET, puis configure le Wi-Fi (section suivante).

## Configuration Wi-Fi (portail web)

Au premier démarrage (aucun Wi-Fi enregistré), l'écran affiche « Connect your phone or computer to
"TRMNL-XXXXXX" Wi-Fi ». Le QR code en haut à droite mène à l'aide en ligne, il ne connecte pas au Wi-Fi.

1. Connecte ton téléphone ou ton ordinateur au réseau ouvert `TRMNL-XXXXXX` (sans mot de passe ;
   `XXXXXX` vient de l'adresse MAC de la carte).
2. La page de configuration s'ouvre toute seule. Sinon, ouvre http://4.3.2.1 dans le navigateur.
3. Choisis ton réseau Wi-Fi (2,4 GHz uniquement sur l'ESP32-S3) et saisis son mot de passe.
   Pour un serveur perso (BYOS), renseigne son URL dans « Advanced settings » → « API server ».
   Sinon, laisse le champ vide pour utiliser trmnl.app.
4. Appuie sur « Connect ». L'appareil redémarre et contacte le serveur.

Le portail reste ouvert 15 minutes. Passé ce délai, l'écran affiche « Wifi Captive Portal timed out »
et la carte se met en veille : appuie sur le bouton (GPIO5) ou sur RESET pour relancer.

Pour revenir au portail plus tard (changer de Wi-Fi), maintiens le bouton GPIO5 enfoncé entre 5 et
15 secondes pendant que la carte se réveille. Au-delà de 15 secondes, la carte efface aussi sa clé
API et se réenregistre auprès du serveur.

Une fois configurée, la carte n'héberge plus de page web : elle dort entre deux rafraîchissements.
Tout se règle ensuite côté serveur (trmnl.app ou BYOS).

## Recompiler

```sh
pio run -e EE04_7inch5_BW
pio run -e EE04_7inch3_Spectra6
```

Les binaires sont produits dans `.pio/build/<env>/`.
