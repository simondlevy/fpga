import neuro
import fpga
import conn

net = neuro.Network()
net.read_from_file("../networks/simple.txt")

proc = fpga.Processor("basys3", "/dev/ttyUSB1", "DIDO")

proc.load_network(net)

conn = conn.Connection("basys3", "/dev/ttyUSB1", "DIDO")
conn.load_network(net)


for _ in range(3):

    proc.clear_activity()
    #conn.clear_activity()

    proc.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])
    #conn.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])

    proc.run(6)
    #conn.run(6)

    print(proc.output_last_fire(0))
    #print(conn.output_last_fire(0))
    #print()
