"""
================================================================================
AUTO-CORRECT & PREDICTIVE TEXT PLAYGROUND (Web Server Launcher)
================================================================================
"""

import http.server
import socketserver
import os
import sys
import webbrowser

if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

PORT = 8000
WEB_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "web")

class ThreadingHTTPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True

class CustomHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def log_message(self, format, *args):
        pass

def run():
    os.chdir(WEB_DIR)
    
    port = PORT
    max_tries = 15
    httpd = None
    
    for attempt in range(max_tries):
        try:
            httpd = ThreadingHTTPServer(("", port), CustomHandler)
            break
        except OSError:
            port += 1
            
    if not httpd:
        print(f"[!] Error: Could not bind to any port from {PORT} to {PORT + max_tries}")
        sys.exit(1)

    url = f"http://localhost:{port}/index.html"
    print("\n" + "=" * 65)
    print(" [*] AUTO-CORRECT & PREDICTIVE TEXT ENGINE - VISUAL PLAYGROUND")
    print("=" * 65)
    print(f" [>] Web Server Running at: {url}")
    print(f" [>] Serving Directory:    {WEB_DIR}")
    print(" [i] Press Ctrl+C in this console to stop the server.")
    print("=" * 65 + "\n")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[*] Server stopped gracefully.")
        httpd.server_close()

if __name__ == "__main__":
    run()
