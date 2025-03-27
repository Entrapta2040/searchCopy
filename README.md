# searchCopy

Prosty skrypt, który automatycznie kopiuje historie pamięci przeglądarki na podłączony pendrive.

## Funkcje
- Kopiuje plik do przygotowanego katalogu na USB
- Obsługuje błędy, gdy pendrive nie jest podłączony (todo)

## Wymagania
- Windows (kopiowanie przystosowane do NTFS)
- Pendrive
- Przeglądarka:
  - Firefox
  - Chrome
  - MS Edge
  - Opera
  - Brave

## Instalacja
1. Pobierz repozytorium:
   ```bash
   git clone https://github.com/Entrapta2040/searchCopy.git
   cd usb-copy-app
   ```
2. Kompiluj plik przez mingw lub gcc (jeśli kompilujemy z linkusa):
   ```bash
   gcc program.c -o nazwa_exe_jaka_chcemy.exe
   ```

## Jak używać?
Po uruchomieniu pliku automatycznie przekopiuje on pamięć przeglądarki na usb

Aby zautomatyzować działanie sugeruje sie:
1. Zdobycie pendrive na którym można emulować CD-ROM
2. W nowej partycji dodajemy nasze exe i robimy plik autorun.inf w którym dajemy:
   ```ini
   [autorun]
   open=program.exe
   ```
3. Przy podłączeniu pendirve powinno automatycznie przekopiować pliki na pendrive
## Licencja
MIT License

