import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('passare il nome del file')
    else:
        NOMEF = sys.argv[1]

        pbuf_rx = {}
        pbuf_tx = {}

        conta = 0

        with open(NOMEF, 'rt') as ing:
            while True:
                riga = ing.readline()
                if len(riga) == 0:
                    break
                conta += 1

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

        print("PBUF RX")
        for k, v in pbuf_rx.items():
            print('\t{} = {}'.format(k, v))
