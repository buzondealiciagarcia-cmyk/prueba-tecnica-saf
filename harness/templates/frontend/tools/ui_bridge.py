#!/usr/bin/env python3
"""Puente MQTT -> Server-Sent Events para el visor (STD-UI UI-TEC-04).

Uso:
  ui_bridge.py --mqtt localhost:1883 --topic <topic del grid>   (en vivo, requiere paho-mqtt)
  ui_bridge.py --replay tests/data/escenario.jsonl              (reproduce una grabación STD-DAT)
Sirve frontend/ en http://localhost:8080 y el flujo en /events. Solo lectura (UI-TEC-03).
"""
import argparse
import logging
import queue
import threading
import time
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

LOG = logging.getLogger("ui")
SUBSCRIBERS = []
LOCK = threading.Lock()


def publish(payload):
    with LOCK:
        for q in list(SUBSCRIBERS):
            try:
                q.put_nowait(payload)
            except queue.Full:
                pass  # el visor solo necesita el último frame


class Handler(SimpleHTTPRequestHandler):
    def do_GET(self):  # noqa: N802 - API de http.server
        if self.path != "/events":
            return super().do_GET()
        q = queue.Queue(maxsize=4)
        with LOCK:
            SUBSCRIBERS.append(q)
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.end_headers()
        try:
            while True:
                data = q.get()
                self.wfile.write(("data: %s\n\n" % data).encode("utf-8"))
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            with LOCK:
                SUBSCRIBERS.remove(q)
        return None

    def log_message(self, fmt, *args):
        LOG.debug(fmt, *args)


def run_replay(path, period_s):
    while True:
        with open(path, encoding="utf-8") as fh:
            for line in fh:
                if line.strip():
                    publish(line.strip())
                    time.sleep(period_s)


def run_mqtt(hostport, topic):
    import paho.mqtt.client as mqtt
    host, _, port = hostport.partition(":")
    cli = mqtt.Client()
    cli.on_message = lambda _c, _u, m: publish(m.payload.decode("utf-8"))
    cli.connect(host, int(port or 1883), keepalive=10)
    cli.subscribe(topic, qos=0)
    cli.loop_forever()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("--mqtt")
    ap.add_argument("--topic")
    ap.add_argument("--replay")
    ap.add_argument("--period-ms", type=float, default=50.0)
    ap.add_argument("--port", type=int, default=8080)
    ap.add_argument("--static", default=str(Path(__file__).resolve().parent.parent / "frontend"))
    args = ap.parse_args()
    logging.basicConfig(level=logging.INFO,
                        format="%(asctime)s.%(msecs)03dZ %(levelname)-6s ui %(message)s",
                        datefmt="%Y-%m-%dT%H:%M:%S")
    logging.Formatter.converter = time.gmtime
    if args.replay:
        threading.Thread(target=run_replay, args=(args.replay, args.period_ms / 1000.0),
                         daemon=True).start()
    elif args.mqtt and args.topic:
        threading.Thread(target=run_mqtt, args=(args.mqtt, args.topic), daemon=True).start()
    else:
        ap.error("indica --replay o --mqtt + --topic")
    srv = ThreadingHTTPServer(("0.0.0.0", args.port), partial(Handler, directory=args.static))
    LOG.info('msg="visor en http://localhost:%d" static=%s', args.port, args.static)
    srv.serve_forever()


if __name__ == "__main__":
    main()
