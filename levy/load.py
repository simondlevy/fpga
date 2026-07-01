import neuro
import fpga

net = neuro.Network()
net.read_from_file("../networks/simple.txt")

proc = fpga.Processor("basys3", "/dev/ttyUSB1", "DIDO")

proc.load_network(net)
