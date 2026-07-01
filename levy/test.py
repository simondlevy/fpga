from importlib import resources
from json import load

import conn
import fpga
import neuro

target_name = "basys3"

target_config = None

with open(resources.files(fpga.config).joinpath("targets.json")) as f:
    target_config = load(f)[target_name]

interface = None

if interface is None or isinstance(interface, str):
    baudrate = 115200
    try:
        baudrate = target_config["parameters"]["uart"]["baud_rates"][-1]
    except KeyError:
        pass
    except IndexError:
        pass
    if isinstance(interface, str):
        interface = Serial(interface, baudrate)
elif isinstance(interface, Serial):
    baudrate = interface.baudrate
else:
    raise RuntimeError("Illegal fpga Processor interface")

clock_freq = target_config["parameters"]["clk_freq"]


net = neuro.Network()
net.read_from_file("../networks/simple.txt")

conn = conn.Connection(net, "basys3", "/dev/ttyUSB1", "DIDO")

for _ in range(3):

    conn.clear_activity()

    conn.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])

    conn.run(6)

    print(conn.output_last_fire(0))
