#!/usr/bin/env python3
"""
@file vulnerable_http_server.py
@brief Serveur HTTP vulnérable pour démonstrations pédagogiques

Ce serveur simule des vulnérabilités HTTP courantes :
- CVE-2015-1635 (HTTP.sys Range DoS) - Simulation
- Buffer overflow dans le parsing des headers
- Command injection dans les paramètres GET

⚠️  DÉMONSTRATION PÉDAGOGIQUE UNIQUEMENT
⚠️  Ne pas utiliser sur des systèmes de production
"""

import socket
import sys
import os
import subprocess
import urllib.parse

class VulnerableHTTPServer:
    def __init__(self, host='0.0.0.0', port=8080):
        self.host = host
        self.port = port
        self.sock = None
        
    def start(self):
        """Démarre le serveur HTTP vulnérable"""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.sock.bind((self.host, self.port))
            self.sock.listen(5)
            print(f"[*] Serveur HTTP vulnérable démarré sur {self.host}:{self.port}")
            print(f"[*] Vulnérabilités simulées:")
            print(f"    - CVE-2015-1635 (Range DoS) - Simulation")
            print(f"    - Buffer overflow dans parsing headers")
            print(f"    - Command injection dans paramètres GET")
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
        """Gère une requête HTTP"""
        try:
            data = client_sock.recv(4096).decode('utf-8', errors='ignore')
            if not data:
                return
            
            print(f"[*] Requête reçue:\n{data[:200]}...\n")
            
            # Parser la requête
            lines = data.split('\r\n')
            if not lines:
                return
            
            request_line = lines[0]
            parts = request_line.split()
            if len(parts) < 2:
                return
            
            method = parts[0]
            path = parts[1]
            
            # Vérifier CVE-2015-1635 (Range DoS)
            if 'Range:' in data:
                range_header = [l for l in lines if l.startswith('Range:')]
                if range_header:
                    range_val = range_header[0]
                    if '18446744073709551615' in range_val or 'bytes=0-' in range_val:
                        print(f"[!] CVE-2015-1635 détectée: Range header malformé")
                        print(f"[!] Simulation DoS (serveur va planter)")
                        # Simulation: envoyer une réponse puis fermer
                        response = "HTTP/1.1 416 Range Not Satisfiable\r\n"
                        response += "Content-Range: bytes */0\r\n"
                        response += "Connection: close\r\n\r\n"
                        client_sock.send(response.encode())
                        client_sock.close()
                        return
            
            # Vérifier command injection dans les paramètres
            if '?' in path:
                path, query = path.split('?', 1)
                params = urllib.parse.parse_qs(query)
                
                # Vulnérabilité: command injection dans le paramètre 'cmd'
                if 'cmd' in params:
                    cmd = params['cmd'][0]
                    print(f"[!] Command injection détectée: {cmd}")
                    # ⚠️  DANGEREUX - Ne jamais faire ça en production
                    try:
                        result = subprocess.run(cmd, shell=True, capture_output=True, timeout=2)
                        output = result.stdout.decode('utf-8', errors='ignore')
                        response = f"HTTP/1.1 200 OK\r\n"
                        response += f"Content-Type: text/plain\r\n"
                        response += f"Content-Length: {len(output)}\r\n"
                        response += f"\r\n{output}"
                        client_sock.send(response.encode())
                    except Exception as e:
                        response = f"HTTP/1.1 500 Internal Server Error\r\n\r\nError: {e}"
                        client_sock.send(response.encode())
                    client_sock.close()
                    return
            
            # Réponse par défaut
            response = "HTTP/1.1 200 OK\r\n"
            response += "Content-Type: text/html\r\n"
            response += "Connection: close\r\n"
            response += "\r\n"
            response += "<html><body><h1>Vulnerable HTTP Server</h1>"
            response += "<p>Ce serveur est vulnérable pour démonstration pédagogique.</p>"
            response += "<p>Vulnérabilités:</p><ul>"
            response += "<li>CVE-2015-1635 (Range DoS) - Simulation</li>"
            response += "<li>Command injection: ?cmd=COMMAND</li>"
            response += "</ul></body></html>"
            
            client_sock.send(response.encode())
            client_sock.close()
            
        except Exception as e:
            print(f"[-] Erreur lors du traitement: {e}")
            if client_sock:
                client_sock.close()

if __name__ == '__main__':
    port = 8080
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    
    server = VulnerableHTTPServer(port=port)
    server.start()

