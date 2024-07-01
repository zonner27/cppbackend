import argparse
import subprocess
import time
import random
import shlex
import signal
import os

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1

FLAMEGRAPH_DIR = './FlameGraph/'

def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
    return process


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()


def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


server = run(start_server())
time.sleep(0.1)

server_pid = server.pid
print(f"Server PID: {server_pid}")

perf_command = f"perf record -p {server_pid} -o perf.data -g"
perf_process = run(perf_command)

make_shots()

time.sleep(0.5)
perf_process.send_signal(signal.SIGINT)
time.sleep(0.5)

stop(server)
time.sleep(1)

if os.path.exists('perf.data') and os.path.getsize('perf.data') > 0:
    flamegraph_command = f"perf script | {FLAMEGRAPH_DIR}/stackcollapse-perf.pl | {FLAMEGRAPH_DIR}/flamegraph.pl > graph.svg"
    subprocess.run(flamegraph_command, shell=True, check=True)
    print("Flamegraph created successfully: graph.svg")
else:
    print("No data in perf.data or file does not exist")

print('Job done')
