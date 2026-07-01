import neuro
import fpga

net = neuro.Network()
net.read_from_file("../networks/simple.txt")

proc = fpga.Processor("basys3", "/dev/ttyUSB1", "DIDO")
proc.load_network(net)

for _ in range(3):
    proc.clear_activity()
    proc.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])
    proc.run(6)
    print(proc.output_last_fire(0))
