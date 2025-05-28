
import math
def calculate_lorawan_airtime(pl_bytes: int, overhead_bytes: int = 13, sf: int = 7, bw: int = 125000, cr: int = 1, preamble_len: float = 13, h: int = 0, de: int = 0) -> float:
    """
    Calculate LoRaWAN airtime in milliseconds.
    
    Parameters:
        pl_bytes: total payload size (App payload + ~13B overhead for LoRaWAN)
        overhead_bytes: overhead size in bytes (default = 13)
        sf: Spreading Factor (e.g., 7)
        bw: Bandwidth in Hz (e.g., 125000)
        cr: Coding Rate offset (e.g., 1 for 4/5)
        preamble_len: number of preamble symbols (default = 8)
        h: Header enabled (0 = yes, 1 = no)
        de: Low data rate optimization (1 = yes, 0 = no)

    Returns:
        Airtime in milliseconds
    """
    # Symbol duration in seconds
    t_sym = (2 ** sf) / bw

    # Add overhead to payload bytes
    pl_bytes += overhead_bytes

    # Payload symbol count
    payload_symb_nb = 8 + max(
        math.ceil(
            (8 * pl_bytes - 4 * sf + 28 + 16 - 20 * h) /
            (4 * (sf - 2 * de))
        ) * (cr + 4),
        0
    )

    # Total airtime in seconds
    t_preamble = (preamble_len + 4.25) * t_sym
    t_payload = payload_symb_nb * t_sym
    t_air = t_preamble + t_payload

    return t_air * 1000  # convert to milliseconds
if __name__ == "__main__":
    # Example usage
    print(calculate_lorawan_airtime(pl_bytes=53, overhead_bytes=13, sf=7, bw=125000, cr=1, preamble_len=8, h=0, de=0))  # Example usage
