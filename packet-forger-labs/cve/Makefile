CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g -fPIC
LDFLAGS = 

# Fichiers CVE (bibliothèques, pas exécutables)
CVE_SOURCES = CVE-2015-1635-http-dos.c \
              CVE-2020-0796-smbghost.c

# Objets compilés
CVE_OBJECTS = $(CVE_SOURCES:.c=.o)

.PHONY: all clean test

all: $(CVE_OBJECTS)

%.o: %.c
	@echo "[*] Compilation: $<"
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "[+] Compilé: $@"

clean:
	@echo "[*] Nettoyage..."
	rm -f $(CVE_OBJECTS)
	@echo "[+] Nettoyage terminé"

test: all
	@echo "[*] Tests des CVEs"
	@echo "[!] Tests à implémenter (futur)"
	@echo "[!] Utilisez les scripts dans ../scripts/ pour tester"

help:
	@echo "Packet Forger Labs - CVEs"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  make          : Compile tous les objets CVE (.o)"
	@echo "  make clean    : Nettoie les objets compilés"
	@echo "  make test     : Lance les tests (futur)"
	@echo "  make help     : Affiche cette aide"
	@echo ""
	@echo "Note: Les CVEs sont compilées en objets (.o) pour utilisation"
	@echo "      comme bibliothèques dans Python ou autres langages."

