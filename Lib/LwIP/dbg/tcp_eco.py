import socket
import struct

SC756_HOST = "10.1.20.1"
SC756_PORT = 7

TCP_MSS = 1460


class TCP_ECO:
    def __init__(self):
        self.sock = None

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((SC756_HOST, SC756_PORT))
            self.sock = s
        except ConnectionRefusedError:
            pass

    def a_posto(self):
        return self.sock is not None

    def eco(self, cosa=None):
        if cosa is None:
            cosa = struct.pack("<I", 0xDEADBEEF)

        scritti = len(cosa)

        try:
            self.sock.sendall(cosa)
            risp = self.sock.recv(scritti)
            if len(risp) < scritti:
                scritti -= len(risp)
                while scritti:
                    tmp = self.sock.recv(scritti)
                    scritti -= len(tmp)
                    risp += tmp
            return risp == cosa

        except OSError:
            return False

    def chiudi(self):
        if self.sock is not None:
            self.sock.close()
            self.sock = None


if __name__ == "__main__":
    import argparse
    import utili

    DESCRIZIONE = """
        Invia eco tcp a SC756 (progetto 492)
        Alternativa a https://github.com/PavelBansky/EchoTool
    """

    argom = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter, description=DESCRIZIONE
    )

    argom.add_argument(
        "-m",
        "--mss",
        type=int,
        default=0,
        help="dimensione in MSS ({})".format(TCP_MSS),
    )
    argom.add_argument("-d", "--dim", type=int, default=100, help="dimensione in bytes")
    argom.add_argument("-q", "--quanti", type=int, default=10, help="numero di echi")

    argom.add_argument(
        '-s', '--server',
        action="store_true",
        help='server eco (False)')

    arghi = argom.parse_args()

    mss = arghi.mss
    if mss < 0:
        mss = -mss
    dim = arghi.dim
    if dim < 0:
        dim = -dim

    if mss == 0 and dim == 0:
        dim = 100

    if mss > 0:
        # vince lui
        dim = mss * TCP_MSS

    crono = utili.CRONOMETRO()

    if arghi.server:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(("10.1.20.254", 7))
        s.listen(1)

        while True:
            conn, addr = s.accept()
            print('connesso a {}'.format(addr))

            tot = 0
            crono.conta()
            try:
                while True:
                    data = conn.recv(TCP_MSS * 10)
                    if not data:
                        break
                    tot += len(data)
                    conn.sendall(data)
            except ConnectionResetError as err:
                print(err)

            durata = crono.durata()
            sdurata = utili.stampaDurata(int(round(durata * 1000.0, 0)))
            tput = round(tot / durata, 1)
            kib = round(tot / (durata * 1024), 1)
            print(
                "Eco: {} in {} ({:.1f} B/s = {:.1f} KiB/s)".format(
                    tot, sdurata, tput, kib
                )
            )
    else:
        eco = TCP_ECO()
        if eco.a_posto():

            quanti = arghi.quanti
            bene = 0
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
                print(
                    "Eco: OK {} in {} ({:.3f} ms = {:.1f} B/s = {:.1f} KiB/s)".format(
                        bene, sdurata, milli, tput, kib
                    )
                )
            else:
                print("Eco: {} su {}".format(bene, arghi.quanti))
