from edalize.flows.vivado import Vivado

files = [
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/risp_neuron.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/risp_synapse.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../networks/5eba9ba2bd.sv"
        },
        {
            "file_type": "verilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/axis_adapter.v"
        },
        {
            "file_type": "verilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/axis_uart.v"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/io_configs.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/dispatch_source.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/stream_sink.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/network_arstn.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/axis_processor.sv"
        },
        {
            "file_type": "systemVerilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/rtl/uart_processor.sv"
        },
        {
            "file_type": "verilogSource",
            "name": "../../../../../../Desktop/fpga/fpga/config/basys3/uart_processor_top.v"
        },
        {
            "file_type": "xdc",
            "name": "../../../../../../Desktop/fpga/fpga/config/basys3/basys3.xdc"
        }
    ]

edam = {
    "files": files,
    "name": "5eba9ba2bd",
    "parameters": {
        "BAUD_RATE": {
            "datatype": "str",
            "default": "4000000",
            "paramtype": "vlogparam"
        },
        "CLK_FREQ": {
            "datatype": "str",
            "default": "100000000.0",
            "paramtype": "vlogparam"
        }
    },
    "tool_options": {
        "vivado": {
            "include_dirs": [
                "../../../../../../Desktop/fpga/fpga/rtl"
            ],
            "part": "xc7a35tcpg236-1",
            "pgm": "vivado",
            "source_mgmt_mode": "All"
        }
    },
    "toplevel": "uart_top"
}

proj_path = "home/levys/.cache/neuro_fpga/eda/basys3/DISO/5eba9ba2bd"

backend = Vivado(edam=edam, work_root=proj_path, verbose=True)
