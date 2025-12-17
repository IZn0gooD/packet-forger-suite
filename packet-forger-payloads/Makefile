CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
LDFLAGS = 

# Détecter json-c (optionnel)
ifneq ($(shell pkg-config --exists json-c 2>/dev/null && echo yes),)
    CFLAGS += -DHAVE_JSON_C $(shell pkg-config --cflags json-c)
    LDFLAGS += $(shell pkg-config --libs json-c)
endif

# Répertoires
SRC_DIR = src
INC_DIR = include
CLI_DIR = cli
OBJ_DIR = obj
BIN_DIR = bin

# Fichiers sources (génération shellcode uniquement)
SOURCES = $(SRC_DIR)/shellcode_generator.c \
          $(SRC_DIR)/asm_generator.c \
          $(SRC_DIR)/polymorphic_shellcode.c \
          $(SRC_DIR)/windows_shellcode_generator.c \
          $(SRC_DIR)/bind_shell.c \
          $(SRC_DIR)/shellcode_encoder.c \
          $(SRC_DIR)/payload_format.c \
          $(SRC_DIR)/verbose_utils.c

# Fichiers objets
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Binaire final
TARGET = $(BIN_DIR)/payload-builder

# Header
HEADER = $(INC_DIR)/payload_generator.h

.PHONY: all clean install test check-deps

all: check-deps $(TARGET)

$(TARGET): $(OBJECTS) $(CLI_DIR)/payload_builder.o | $(BIN_DIR)
	@echo "[*] Édition des liens..."
	$(CC) $(OBJECTS) $(CLI_DIR)/payload_builder.o -o $(TARGET) $(LDFLAGS)
	@echo "[+] Compilation réussie : $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER) | $(OBJ_DIR)
	@echo "[*] Compilation : $<"
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(CLI_DIR)/payload_builder.o: $(CLI_DIR)/payload_builder.c $(HEADER) | $(OBJ_DIR)
	@echo "[*] Compilation CLI : $<"
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

clean:
	@echo "[*] Nettoyage..."
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(CLI_DIR)/*.o
	rm -f output/*.bin output/*.json
	@echo "[+] Nettoyage terminé"

install: $(TARGET)
	@echo "[*] Installation..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "[+] Installation terminée"

check-deps:
	@echo "[*] Vérification des dépendances..."
	@which nasm > /dev/null 2>&1 && echo "[+] NASM trouvé: $$(nasm --version | head -1)" || echo "[-] NASM non trouvé (requis pour --asm et --polymorphic)"
	@which objcopy > /dev/null 2>&1 && echo "[+] objcopy trouvé" || echo "[-] objcopy non trouvé (requis pour --asm)"
	@which objdump > /dev/null 2>&1 && echo "[+] objdump trouvé" || echo "[-] objdump non trouvé (requis pour --asm)"
	@pkg-config --exists json-c && echo "[+] json-c trouvé" || echo "[-] json-c non trouvé (requis pour metadata JSON)"
	@echo ""

test: $(TARGET)
	@echo "[*] Tests de génération"
	@echo "[!] Tests à implémenter (futur)"

# Aide
help:
	@echo "Packet Forger Payloads - Makefile"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  make          : Compile le générateur de payloads"
	@echo "  make clean    : Nettoie les fichiers de compilation"
	@echo "  make install : Installe le binaire (nécessite sudo)"
	@echo "  make test     : Lance les tests (futur)"
	@echo "  make check-deps : Vérifie les dépendances"
	@echo "  make help     : Affiche cette aide"

