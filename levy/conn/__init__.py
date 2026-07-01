import neuro

from periphery import Serial

class Connection:

    def __init__(
        self,
        target: str,
        interface: Serial | str | None = None,
        io_type: str = "DISO",
        *args,
        **kwargs,
    ):
 
        pass

    def clear_activity(self) -> None:

        pass

    def apply_spikes(self, spikes: list[neuro.Spike]) -> None:

        pass

    def run(self, time: int) -> None:

        pass

    def output_last_fire(self, out_idx: int) -> float:

        return 0
