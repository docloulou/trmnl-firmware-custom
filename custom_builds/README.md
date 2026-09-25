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
3. Appuie sur RESET. L'écran affiche le QR code de configuration Wi-Fi : connecte-toi au point
   d'accès « TRMNL » et renseigne ton Wi-Fi et ton serveur.

## Recompiler

```sh
pio run -e EE04_7inch5_BW
pio run -e EE04_7inch3_Spectra6
```

Les binaires sont produits dans `.pio/build/<env>/`.
