import sys
import queue

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('passare il nome del file')
    else:
        NOMEF = sys.argv[1]

        pbuf_rx = {}
        pbuf_tx = {}

        max_lstRx = 0
        lstRx = queue.SimpleQueue()


        def svuota(coda: queue.SimpleQueue):
            while not coda.empty():
                coda.get_nowait()


        def ctrl_estrazione(coda: queue.SimpleQueue, cosa, dim) -> bool:
            esito = True
            if coda.empty():
                esito = False
                print("estrazione da coda vuota")
            else:
                elem = coda.get_nowait()
                if elem != cosa:
                    esito = False
                    print("errore estrazione: {} invece di {}".format(elem, cosa))
                if dim != coda.qsize():
                    esito = False
                    print("errore estrazione: {} invece di {}".format(coda.qsize(), dim))

            return esito


        def ctrl_inserimento(coda: queue.SimpleQueue, cosa, dim) -> bool:
            coda.put_nowait(cosa)

            if dim != coda.qsize():
                print("errore inserimento: {} invece di {}".format(coda.qsize(), dim))
                return False

            return True


        def togli_diario(riga):
            riga = riga.strip()
            meno = riga.find('-')
            if meno == -1:
                return riga

            return riga[meno + 1:]


        conta = 0

        with open(NOMEF, 'rt') as ing:
            while True:
                riga = ing.readline()
                if len(riga) == 0:
                    break
                conta += 1

                if 'lst rx iniz' in riga:
                    svuota(lstRx)
                    continue

                if 'lst rx' in riga:
                    riga = togli_diario(riga)
                    elem = riga.split('->')
                    if len(elem) == 2:
                        if 'lst rx' in elem[0]:
                            # estrazione
                            elem = elem[1].split('=')
                            if len(elem) == 2:
                                cosa = elem[0].strip()
                                dim = int(elem[1].strip())

                                if not ctrl_estrazione(lstRx, cosa, dim):
                                    print('{}) errore estra'.format(conta))
                            else:
                                print('{}) {} invece di 2'.format(conta, len(elem)))
                        elif 'lst rx' in elem[1]:
                            # inserimento
                            cosa = elem[0].strip()
                            elem = elem[1].split('=')
                            if len(elem) == 2:
                                dim = int(elem[1].strip())
                                if not ctrl_inserimento(lstRx, cosa, dim):
                                    print('{}) errore inser'.format(conta))
                                elif dim > max_lstRx:
                                    max_lstRx = dim
                            else:
                                print('{}) {} invece di 2'.format(conta, len(elem)))

                        else:
                            print('{}) errore'.format(conta))
                    else:
                        print('{}) {} invece di 2'.format(conta, len(elem)))
                    continue

                if 'pbuf_rx_alloc' in riga:
                    # %08X = pbuf_rx_alloc
                    elem = riga.strip().split(' ')
                    if len(elem) == 3:
                        if elem[0] not in pbuf_rx:
                            pbuf_rx[elem[0]] = 1
                        else:
                            pbuf_rx[elem[0]] += 1
                    else:
                        print('{}) {} invece di 3'.format(conta, len(elem)))
                    continue

                if 'pbuf_rx_free' in riga:
                    # pbuf_rx_free %08X
                    elem = riga.strip().split(' ')
                    if len(elem) == 2:
                        if elem[1] not in pbuf_rx:
                            pbuf_rx[elem[1]] = 0
                        else:
                            pbuf_rx[elem[1]] -= 1
                    else:
                        print('{}) {} invece di 2'.format(conta, len(elem)))
                    continue

                if 'pbuf_tx_alloc' in riga:
                    elem = riga.strip().split(' ')
                    if len(elem) == 3:
                        if elem[0] not in pbuf_tx:
                            pbuf_tx[elem[0]] = 1
                        else:
                            pbuf_tx[elem[0]] += 1
                    else:
                        print('{}) {} invece di 3'.format(conta, len(elem)))
                    continue

                if 'pbuf_tx_free' in riga:
                    elem = riga.strip().split(' ')
                    if len(elem) == 2:
                        if elem[1] not in pbuf_tx:
                            pbuf_tx[elem[1]] = 0
                        else:
                            pbuf_tx[elem[1]] -= 1
                    else:
                        print('{}) {} invece di 2'.format(conta, len(elem)))
                    continue

        print("Max dim lista rx {}".format(max_lstRx))

        print("PBUF RX")
        for k, v in pbuf_rx.items():
            print('\t{} = {}'.format(k, v))
