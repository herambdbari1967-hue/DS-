import urllib.request
import sys

endpoints = [
    "http://localhost:8000/index.html",
    "http://localhost:8000/style.css",
    "http://localhost:8000/trie_engine.js",
    "http://localhost:8000/dict_data.js",
    "http://localhost:8000/app.js"
]

all_ok = True
for url in endpoints:
    try:
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=5) as response:
            data = response.read()
            print(f"[OK] {url} -> Status: {response.status}, Size: {len(data):,} bytes")
    except Exception as e:
        print(f"[FAIL] {url} -> Error: {e}")
        all_ok = False

if all_ok:
    print("\nAll web assets verified and responding successfully over HTTP.")
else:
    print("\nSome endpoints failed.")
    sys.exit(1)
