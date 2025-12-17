#!/usr/bin/env python3
"""
@file vulnerable_smb_server.py
@brief Serveur SMB vulnérable pour démonstrations pédagogiques

Ce serveur simule des vulnérabilités SMB courantes :
- CVE-2020-0796 (SMBGhost) - Simulation
- Buffer overflow dans la décompression SMBv3

⚠️  DÉMONSTRATION PÉDAGOGIQUE UNIQUEMENT
⚠️  Ne pas utiliser sur des systèmes de production
"""

import socket
import sys
import struct

class VulnerableSMBServer:
    def __init__(self, host='0.0.0.0', port=445):
        self.host = host
        self.port = port
        self.sock = None
        
    def start(self):
        """Démarre le serveur SMB vulnérable"""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.sock.bind((self.host, self.port))
            self.sock.listen(5)
            print(f"[*] Serveur SMB vulnérable démarré sur {self.host}:{self.port}")
            print(f"[*] Vulnérabilités simulées:")
            print(f"    - CVE-2020-0796 (SMBGhost) - Simulation")
            print(f"    - Buffer overflow dans décompression SMBv3")
            print(f"[*] Appuyez sur Ctrl+C pour arrêter\n")
            
            while True:
                client_sock, addr = self.sock.accept()
                print(f"[+] Connexion reçue de {addr[0]}:{addr[1]}")
                self.handle_request(client_sock, addr)
                
        except KeyboardInterrupt:
            print("\n[*] Arrêt du serveur...")
        except Exception as e:
            print(f"[-] Erreur: {e}")
        finally:
            if self.sock:
                self.sock.close()
    
    def handle_request(self, client_sock, addr):
        """Gère une requête SMB"""
        try:
            # Lire le NetBIOS Session Service Header
            nbss_header = client_sock.recv(4)
            if len(nbss_header) < 4:
                return
            
            # Type de message (premier byte)
            msg_type = nbss_header[0]
            if msg_type != 0x00:  # Session Message
                return
            
            # Longueur du message SMB (3 bytes big-endian)
            smb_length = struct.unpack('>I', b'\x00' + nbss_header[1:4])[0]
            
            # Lire le message SMB
            smb_data = client_sock.recv(smb_length)
            if len(smb_data) < 4:
                return
            
            # Vérifier le protocole SMB
            if smb_data[:4] == b'\xFE\x53\x4D\x42':  # SMB2
                print(f"[*] Requête SMB2 reçue")
                self.handle_smb2(client_sock, smb_data)
            elif smb_data[:4] == b'\xFF\x53\x4D\x42':  # SMB1
                print(f"[*] Requête SMB1 reçue")
                self.handle_smb1(client_sock, smb_data)
            else:
                print(f"[*] Protocole inconnu")
            
            client_sock.close()
            
        except Exception as e:
            print(f"[-] Erreur lors du traitement: {e}")
            if client_sock:
                client_sock.close()
    
    def handle_smb2(self, client_sock, smb_data):
        """Gère une requête SMB2"""
        if len(smb_data) < 64:
            return
        
        # Lire le header SMB2 (64 bytes)
        header = smb_data[:64]
        
        # Command (offset 12-13)
        command = struct.unpack('<H', header[12:14])[0]
        
        # Vérifier CVE-2020-0796 (COMPRESSION_TRANSFORM = 0x03)
        if command == 0x03:  # COMPRESSION_TRANSFORM
            print(f"[!] CVE-2020-0796 détectée: Compression Transform")
            print(f"[!] Simulation buffer overflow (serveur va planter)")
            
            # Lire le Compression Transform Header
            if len(smb_data) >= 76:
                # OriginalCompressedSegmentSize (offset 64-67)
                original_size = struct.unpack('<I', smb_data[64:68])[0]
                print(f"[!] OriginalCompressedSegmentSize: {original_size} (0x{original_size:08x})")
                
                if original_size > 0x1000000:  # > 16MB
                    print(f"[!] Taille décompressée trop grande - Buffer overflow simulé")
                    # Simulation: envoyer une erreur puis fermer
                    response = self.build_smb2_error(0xC000000D)  # STATUS_INVALID_PARAMETER
                    client_sock.send(response)
                    return
        
        # Réponse par défaut (NEGOTIATE)
        response = self.build_smb2_negotiate_response()
        client_sock.send(response)
    
    def handle_smb1(self, client_sock, smb_data):
        """Gère une requête SMB1"""
        # Réponse SMB1 basique
        response = b'\xFF\x53\x4D\x42'  # Protocol
        response += b'\x72'  # Command: NEGOTIATE
        response += b'\x00\x00\x00\x00'  # Status
        response += b'\x00'  # Flags
        response += b'\x00\x00'  # Flags2
        response += b'\x00\x00'  # PID High
        response += b'\x00' * 8  # Signature
        response += b'\x00\x00'  # Reserved
        response += b'\x00\x00'  # TID
        response += b'\x00\x00'  # PID
        response += b'\x00\x00'  # UID
        response += b'\x00\x00'  # MID
        
        client_sock.send(response)
    
    def build_smb2_negotiate_response(self):
        """Construit une réponse SMB2 NEGOTIATE"""
        # NetBIOS Session Service Header
        nbss = b'\x00'  # Session Message
        nbss += b'\x00\x00\x64'  # Length: 100 bytes
        
        # SMB2 Header
        smb2 = b'\xFE\x53\x4D\x42'  # Protocol
        smb2 += b'\x40\x00'  # Header Length: 64
        smb2 += b'\x00\x00'  # Credit Charge
        smb2 += b'\x00\x00\x00\x00'  # Status: SUCCESS
        smb2 += b'\x00\x00'  # Command: (sera rempli)
        smb2 += b'\x00\x00'  # Credits
        smb2 += b'\x00\x00\x00\x00'  # Flags
        smb2 += b'\x00\x00\x00\x00'  # Next Command
        smb2 += b'\x00' * 8  # Message ID
        smb2 += b'\x00\x00'  # Process ID
        smb2 += b'\x00\x00'  # Tree ID
        smb2 += b'\x00' * 8  # Session ID
        smb2 += b'\x00' * 16  # Signature
        
        # NEGOTIATE Response (simplifié)
        negotiate = b'\x00\x00'  # StructureSize
        negotiate += b'\x00'  # SecurityMode
        negotiate += b'\x00'  # DialectRevision
        negotiate += b'\x00' * 16  # ServerGuid
        negotiate += b'\x00\x00\x00\x00'  # Capabilities
        negotiate += b'\x00\x00\x00\x00'  # MaxTransactSize
        negotiate += b'\x00\x00\x00\x00'  # MaxReadSize
        negotiate += b'\x00\x00\x00\x00'  # MaxWriteSize
        negotiate += b'\x00' * 8  # SystemTime
        negotiate += b'\x00' * 8  # ServerStartTime
        negotiate += b'\x00\x00'  # SecurityBufferOffset
        negotiate += b'\x00\x00'  # SecurityBufferLength
        negotiate += b'\x00\x00\x00\x00'  # Reserved
        
        return nbss + smb2 + negotiate
    
    def build_smb2_error(self, status):
        """Construit une réponse SMB2 avec erreur"""
        nbss = b'\x00\x00\x00\x40'  # Session Message, 64 bytes
        
        smb2 = b'\xFE\x53\x4D\x42'  # Protocol
        smb2 += b'\x40\x00'  # Header Length
        smb2 += b'\x00\x00'  # Credit Charge
        smb2 += struct.pack('<I', status)  # Status (erreur)
        smb2 += b'\x00\x00'  # Command
        smb2 += b'\x00\x00'  # Credits
        smb2 += b'\x00\x00\x00\x00'  # Flags
        smb2 += b'\x00\x00\x00\x00'  # Next Command
        smb2 += b'\x00' * 8  # Message ID
        smb2 += b'\x00\x00'  # Process ID
        smb2 += b'\x00\x00'  # Tree ID
        smb2 += b'\x00' * 8  # Session ID
        smb2 += b'\x00' * 16  # Signature
        
        return nbss + smb2

if __name__ == '__main__':
    port = 445
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    
    server = VulnerableSMBServer(port=port)
    server.start()

