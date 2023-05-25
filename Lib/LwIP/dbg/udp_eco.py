import socket
import struct

SC756_HOST = "10.1.20.1"
SC756_PORT = 7

TCP_MSS = 1460
NET_DESC = 4


class UDP_ECO:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def a_posto(self):
        return self.sock is not None

    def eco(self, cosa=None):
        if cosa is None:
            cosa = struct.pack('<I', 0xDEADBEEF)

        dim = len(cosa)

        try:
            self.sock.sendto(cosa, (SC756_HOST, SC756_PORT))
            risp = self.sock.recvfrom(dim)[0]
            return risp == cosa
        except OSError as msg:
            return False

    def chiudi(self):
        if self.sock is not None:
            self.sock.close()
            self.sock = None


if __name__ == '__main__':
    import argparse
    import utili

    DESCRIZIONE = '''
        Invia eco udp a SC756 (progetto 492)
        Alternativa a https://github.com/PavelBansky/EchoTool
    '''

    argom = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=DESCRIZIONE)

    argom.add_argument(
        '-d', '--dimensione',
        type=int,
        default=TCP_MSS,
        help="dimensione dell'eco")

    argom.add_argument(
        '-q', '--quanti',
        type=int,
        default=10,
        help='numero di echi')

    arghi = argom.parse_args()

    eco = UDP_ECO()
    if eco.a_posto():
        crono = utili.CRONOMETRO()
        quanti = arghi.quanti
        bene = 0
        dim = arghi.dimensione
        if dim <= NET_DESC:
            dim *= TCP_MSS
        if dim > NET_DESC * TCP_MSS:
            dim = NET_DESC * TCP_MSS

        dati1 = utili.byte_casuali(dim)
        dati2 = utili.byte_casuali(dim)
        dati = dati1

        crono.conta()
        while quanti:
            if eco.eco(dati):
                bene += 1
            quanti -= 1
            if dati is dati1:
                dati = dati2
            else:
                dati = dati1

        durata = crono.durata()
        eco.chiudi()

        sdurata = utili.stampaDurata(int(round(durata * 1000.0, 0)))
        if bene == arghi.quanti:
            milli = round(1000.0 * durata / bene, 3)
            tput = round((dim * bene) / durata, 1)
            kib = round((dim * bene) / (durata * 1024), 1)
            print("Eco: OK {} in {} ({:.3f} ms = {:.1f} B/s = {:.1f} KiB/s)".format(bene, sdurata, milli, tput, kib))
        else:
            print('Eco: {} su {}'.format(bene, arghi.quanti))
