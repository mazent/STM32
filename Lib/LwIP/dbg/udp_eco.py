import socket
import struct

SC756_HOST = "10.1.20.1"
SC756_PORT = 7

TCP_MSS = 1460
NET_DESC = 4


class UDP_ECO:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        if self.sock is not None:
            self.sock.settimeout(0.5)

    def a_posto(self):
        return self.sock is not None

    def eco(self, cosa=None):
        if cosa is None:
            cosa = struct.pack("<I", 0xDEADBEEF)

        scritti = len(cosa)

        try:
            self.sock.sendto(cosa, (SC756_HOST, SC756_PORT))
            risp = self.sock.recvfrom(scritti)[0]
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
        Invia eco udp a SC756 (progetto 492)
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
    argom.add_argument(
        "-q",
        "--quanti",
        type=int,
        default=10,
        help="numero di echi (negativo -> fine per errore)",
    )

    argom.add_argument("-s", "--server", action="store_true", help="server eco (False)")

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

    if arghi.server:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("10.1.20.254", 7))

        while True:
            dati, rem = s.recvfrom(TCP_MSS * 10)
            if not dati:
                break
            s.sendto(dati, rem)
    else:
        crono = utili.CRONOMETRO()

        eco = UDP_ECO()

        def fammene(enne):
            bene = 0
            quanti = enne

            dati1 = utili.byte_casuali(dim)
            dati2 = utili.byte_casuali(dim)
            dati = dati1

            uscito = False
            try:
                crono.conta()
                while quanti:
                    if eco.eco(dati):
                        bene += 1
                    quanti -= 1
                    if dati is dati1:
                        dati = dati2
                    else:
                        dati = dati1
            except KeyboardInterrupt:
                uscito = True
            durata = crono.durata()
            eco.chiudi()

            if uscito:
                print("Ne ho fatti {} poi hai interrotto".format(bene))
            else:
                print("Ne ho fatti {} su {} ".format(bene, enne))
            if bene:
                sdurata = utili.stampaDurata(int(round(durata * 1000.0, 0)))
                milli = round(1000.0 * durata / bene, 3)
                tput = round((dim * bene) / durata, 1)
                kib = round((dim * bene) / (durata * 1024), 1)
                print(
                    "Eco: OK {} in {} ({:.3f} ms = {:.1f} B/s = {:.1f} KiB/s)".format(
                        bene, sdurata, milli, tput, kib
                    )
                )

        def esci_per_errore(errori):
            bene = 0
            quanti = errori

            dati1 = utili.byte_casuali(dim)
            dati2 = utili.byte_casuali(dim)
            dati = dati1

            uscito = False
            try:
                crono.conta()
                while quanti:
                    if eco.eco(dati):
                        bene += 1
                    else:
                        quanti -= 1
                        if quanti == 0:
                            break
                    if dati is dati1:
                        dati = dati2
                    else:
                        dati = dati1
            except KeyboardInterrupt:
                uscito = True
            durata = crono.durata()
            eco.chiudi()

            if uscito:
                print("Ne ho fatti {} poi hai interrotto".format(bene))
            else:
                print("Ne ho fatti {} con {} errori".format(bene, errori))
            if bene:
                sdurata = utili.stampaDurata(int(round(durata * 1000.0, 0)))
                milli = round(1000.0 * durata / bene, 3)
                tput = round((dim * bene) / durata, 1)
                kib = round((dim * bene) / (durata * 1024), 1)
                print(
                    "Eco: OK {} in {} ({:.3f} ms = {:.1f} B/s = {:.1f} KiB/s)".format(
                        bene, sdurata, milli, tput, kib
                    )
                )

        if eco.a_posto():
            quanti = arghi.quanti

            if quanti > 0:
                fammene(quanti)
            elif quanti < 0:
                esci_per_errore(-quanti)
            else:
                print("parametro errato")
