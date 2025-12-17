CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
LDFLAGS =

# Répertoires
SRC_DIR = src
BIN_DIR = bin

# Fichiers sources
SOURCES = $(SRC_DIR)/main.c

# Binaire final
TARGET = $(BIN_DIR)/packet-forger

.PHONY: all clean install test help

all: $(TARGET)

$(TARGET): $(SOURCES) | $(BIN_DIR)
	@echo "[*] Compilation du CLI unifié..."
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "[+] Compilation réussie : $(TARGET)"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

clean:
	@echo "[*] Nettoyage..."
	rm -rf $(BIN_DIR)
	@echo "[+] Nettoyage terminé"

install: $(TARGET)
	@echo "[*] Installation..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "[+] Binaire installé : /usr/local/bin/packet-forger"
	@echo "[*] Installation des scripts de completion..."
	@if [ -d /etc/bash_completion.d ]; then \
		sudo cp completions/packet-forger.bash /etc/bash_completion.d/ && \
		echo "[+] Completion Bash installée : /etc/bash_completion.d/packet-forger.bash"; \
	fi
	@if [ -d /usr/local/share/zsh/site-functions ]; then \
		sudo cp completions/_packet-forger /usr/local/share/zsh/site-functions/ && \
		echo "[+] Completion Zsh installée : /usr/local/share/zsh/site-functions/_packet-forger"; \
	elif [ -d ~/.zsh/completions ]; then \
		mkdir -p ~/.zsh/completions && \
		cp completions/_packet-forger ~/.zsh/completions/ && \
		echo "[+] Completion Zsh installée : ~/.zsh/completions/_packet-forger"; \
		echo "[!] Ajoutez 'fpath=(~/.zsh/completions \$fpath)' à votre ~/.zshrc"; \
	fi
	@echo "[+] Installation terminée"

test: $(TARGET)
	@echo "[*] Tests du CLI unifié..."
	@echo ""
	@echo "Test 1 : Aide principale"
	@$(TARGET) --help
	@echo ""
	@echo "Test 2 : Version"
	@$(TARGET) --version
	@echo ""
	@echo "Test 3 : Aide module lab"
	@$(TARGET) lab --help || true
	@echo ""
	@echo "[+] Tests terminés"

help:
	@echo "Packet Forger CLI - Makefile"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  make          : Compile le CLI unifié"
	@echo "  make clean    : Nettoie les fichiers de compilation"
	@echo "  make install  : Installe le binaire (nécessite sudo)"
	@echo "  make test     : Lance les tests du CLI"
	@echo "  make help     : Affiche cette aide"

