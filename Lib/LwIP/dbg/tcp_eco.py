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
            self.sock.settimeout(0.5)
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
    import sys
    import threading

    DESCRIZIONE = """
        Invia eco tcp 
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
    argom.add_argument(
        "-t",
        "--tempo",
        type=int,
        default=0,
        help="durata della prova in minuti",
    )

    argom.add_argument("-s", "--server", action="store_true", help="server eco (False)")

    arghi = argom.parse_args()

    mss = arghi.mss
    if mss < 0:
        sys.exit("Errore: mss non valido")
    dim = arghi.dim
    if dim < 0:
        sys.exit("Errore: dim non valido")
    tempo = arghi.tempo
    if tempo < 0:
        sys.exit("Errore: tempo non valido")
    quanti = arghi.quanti
    if tempo == 0 and quanti == 0:
        sys.exit("Errore: tempo/quanti = zero")
    if tempo > 0:
        quanti = 0
    if mss == 0 and dim == 0:
        sys.exit("Errore: dim/mss = zero")

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
            print("connesso a {}".format(addr))

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


        def esci_a_tempo(tempo):
            bene = 0
            male = 0

            dati1 = utili.byte_casuali(dim)
            dati2 = utili.byte_casuali(dim)
            dati = dati1

            esci = threading.Event()

            def fine():
                esci.set()

            scade = utili.Periodico(fine)
            try:
                scade.avvia(tempo * 60)
                crono.conta()
                while not esci.is_set():
                    if eco.eco(dati):
                        bene += 1
                    else:
                        male += 1

                    if dati is dati1:
                        dati = dati2
                    else:
                        dati = dati1
            except (KeyboardInterrupt, utili.PROBLEMA):
                pass
            durata = crono.durata()
            eco.chiudi()
            scade.termina()

            print("Finito con {} errori".format(male))
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
            if quanti > 0:
                fammene(quanti)
            elif quanti < 0:
                esci_per_errore(-quanti)
            else:
                esci_a_tempo(tempo)
